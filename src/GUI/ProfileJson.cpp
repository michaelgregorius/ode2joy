#include "ProfileJson.h"

#include <QJsonArray>

#include <algorithm>
#include <cstddef>

namespace
{

int const fileVersion = 2;

template <typename E>
struct Name
{
    E value;
    char const *name;
};

Name<ControlType> const controlTypes[] = {
    { ControlType::Axis, "axis" }, { ControlType::Button, "button" }, { ControlType::Hat, "hat" } };
Name<HatComponent> const hatComponents[] = {
    { HatComponent::X, "x" }, { HatComponent::Y, "y" }, { HatComponent::Up, "up" },
    { HatComponent::Right, "right" }, { HatComponent::Down, "down" }, { HatComponent::Left, "left" } };
Name<MidiTargetKind> const targetKinds[] = {
    { MidiTargetKind::ControlChange, "cc" }, { MidiTargetKind::Note, "note" },
    { MidiTargetKind::PitchBend, "pitchBend" }, { MidiTargetKind::ProgramChange, "programChange" },
    { MidiTargetKind::ChannelAftertouch, "aftertouch" }, { MidiTargetKind::Mmc, "mmc" } };
Name<CurveType> const curveTypes[] = {
    { CurveType::Linear, "linear" }, { CurveType::Exponential, "exponential" },
    { CurveType::Logarithmic, "logarithmic" } };
Name<AxisRange> const axisRanges[] = {
    { AxisRange::Bipolar, "bipolar" }, { AxisRange::Unipolar, "unipolar" } };

template <typename E, size_t N>
QString toName(Name<E> const (&table)[N], E value)
{
    for (auto const &n : table)
    {
        if (n.value == value)
        {
            return QString::fromLatin1(n.name);
        }
    }
    return QString();
}

template <typename E, size_t N>
bool fromName(Name<E> const (&table)[N], QString const &name, E &value)
{
    for (auto const &n : table)
    {
        if (name == QLatin1String(n.name))
        {
            value = n.value;
            return true;
        }
    }
    return false;
}

uint8_t clampByte(int v)
{
    return static_cast<uint8_t>(std::max(0, std::min(127, v)));
}

QString str(std::string const &s)
{
    return QString::fromStdString(s);
}

} // namespace

QJsonObject profileToJson(Profile const &profile)
{
    QJsonArray mappings;
    for (Mapping const &m : profile.mappings)
    {
        QJsonObject source;
        source["type"] = toName(controlTypes, m.source.type);
        source["index"] = m.source.index;
        if (m.source.type == ControlType::Hat)
        {
            source["component"] = toName(hatComponents, m.hatComponent);
        }

        QJsonObject target;
        target["kind"] = toName(targetKinds, m.target.kind);
        target["channel"] = m.target.channel + 1;
        target["number"] = m.target.number;
        target["deviceId"] = m.target.deviceId;
        target["onValue"] = m.target.onValue;
        target["offValue"] = m.target.offValue;

        QJsonObject curve;
        curve["type"] = toName(curveTypes, m.curve.type);
        curve["amount"] = m.curve.amount;

        QJsonObject o;
        o["source"] = source;
        o["target"] = target;
        o["range"] = toName(axisRanges, m.range);
        o["curve"] = curve;
        o["deadZone"] = m.deadZone;
        o["smoothingMs"] = m.smoothing;
        o["invert"] = m.invert;
        o["maxRateHz"] = m.maxRateHz;
        mappings.append(o);
    }

    QJsonArray sticks;
    for (Stick const &s : profile.sticks)
    {
        QJsonObject o;
        o["label"] = str(s.label);
        o["xAxis"] = s.xAxis;
        o["yAxis"] = s.yAxis;
        o["radialDeadZone"] = s.radialDeadZone;
        sticks.append(o);
    }

    QJsonArray labels;
    for (ControlLabel const &l : profile.labels)
    {
        QJsonObject o;
        o["type"] = toName(controlTypes, l.type);
        o["index"] = l.index;
        o["text"] = str(l.text);
        labels.append(o);
    }

    QJsonObject root;
    root["version"] = fileVersion;
    root["name"] = str(profile.name);
    root["deviceGuid"] = str(profile.deviceGuid);
    root["mappings"] = mappings;
    root["sticks"] = sticks;
    root["labels"] = labels;
    return root;
}

bool profileFromJson(QJsonObject const &json, Profile &profile, QString &error)
{
    if (json["version"].toInt() != fileVersion)
    {
        error = QObject::tr("Unsupported profile version");
        return false;
    }

    Profile p;
    p.name = json["name"].toString().toStdString();
    p.deviceGuid = json["deviceGuid"].toString().toStdString();

    QJsonArray const mappings = json["mappings"].toArray();
    for (int i = 0; i < mappings.size(); ++i)
    {
        QJsonObject const o = mappings[i].toObject();
        QJsonObject const source = o["source"].toObject();
        QJsonObject const target = o["target"].toObject();
        QJsonObject const curve = o["curve"].toObject();

        Mapping m;
        if (!fromName(controlTypes, source["type"].toString(), m.source.type)
            || !fromName(targetKinds, target["kind"].toString(), m.target.kind))
        {
            error = QObject::tr("Invalid source or target type in mapping %1").arg(i + 1);
            return false;
        }

        m.source.index = std::max(0, source["index"].toInt());
        if (m.source.type == ControlType::Hat
            && !fromName(hatComponents, source["component"].toString(), m.hatComponent))
        {
            error = QObject::tr("Invalid hat component in mapping %1").arg(i + 1);
            return false;
        }

        m.target.channel = static_cast<uint8_t>(std::max(0, std::min(15, target["channel"].toInt(1) - 1)));
        m.target.number = clampByte(target["number"].toInt());
        m.target.deviceId = std::min<uint8_t>(127, clampByte(target["deviceId"].toInt(127)));
        m.target.onValue = clampByte(target["onValue"].toInt(127));
        m.target.offValue = clampByte(target["offValue"].toInt(0));

        fromName(axisRanges, o["range"].toString(), m.range);
        fromName(curveTypes, curve["type"].toString(), m.curve.type);
        m.curve.amount = static_cast<float>(curve["amount"].toDouble(3.0));
        m.deadZone = static_cast<float>(std::max(0.0, std::min(0.99, o["deadZone"].toDouble())));
        m.smoothing = static_cast<float>(std::max(0.0, o["smoothingMs"].toDouble()));
        m.invert = o["invert"].toBool();
        m.maxRateHz = static_cast<float>(std::max(0.0, o["maxRateHz"].toDouble(250.0)));

        p.mappings.push_back(m);
    }

    for (QJsonValue const v : json["sticks"].toArray())
    {
        QJsonObject const o = v.toObject();
        Stick s;
        s.label = o["label"].toString().toStdString();
        s.xAxis = std::max(0, o["xAxis"].toInt());
        s.yAxis = std::max(0, o["yAxis"].toInt(1));
        s.radialDeadZone = static_cast<float>(std::max(0.0, std::min(0.99, o["radialDeadZone"].toDouble())));
        p.sticks.push_back(s);
    }

    for (QJsonValue const v : json["labels"].toArray())
    {
        QJsonObject const o = v.toObject();
        ControlLabel l;
        if (fromName(controlTypes, o["type"].toString(), l.type))
        {
            l.index = std::max(0, o["index"].toInt());
            l.text = o["text"].toString().toStdString();
            p.labels.push_back(l);
        }
    }

    profile = p;
    return true;
}

QJsonObject recordToJson(TypeRecord const &record)
{
    QJsonObject type;
    type["guid"] = str(record.type.guid);
    type["name"] = str(record.type.name);
    type["numAxes"] = record.type.numAxes;
    type["numButtons"] = record.type.numButtons;
    type["numHats"] = record.type.numHats;
    type["leftStickX"] = record.type.leftStickX;
    type["leftStickY"] = record.type.leftStickY;
    type["rightStickX"] = record.type.rightStickX;
    type["rightStickY"] = record.type.rightStickY;

    QJsonArray presets;
    for (Profile const &p : record.presets)
    {
        presets.append(profileToJson(p));
    }

    QJsonObject instances;
    for (auto const &entry : record.instancePresets)
    {
        instances[str(entry.first)] = str(entry.second);
    }

    QJsonObject root;
    root["version"] = fileVersion;
    root["type"] = type;
    root["activePreset"] = str(record.activePreset);
    root["presets"] = presets;
    root["instancePresets"] = instances;
    return root;
}

bool recordFromJson(QJsonObject const &json, TypeRecord &record, QString &error)
{
    if (json["version"].toInt() != fileVersion)
    {
        error = QObject::tr("Unsupported file version");
        return false;
    }

    QJsonObject const type = json["type"].toObject();
    TypeRecord r;
    r.type.guid = type["guid"].toString().toStdString();
    r.type.name = type["name"].toString().toStdString();
    r.type.numAxes = std::max(0, type["numAxes"].toInt());
    r.type.numButtons = std::max(0, type["numButtons"].toInt());
    r.type.numHats = std::max(0, type["numHats"].toInt());
    r.type.leftStickX = type["leftStickX"].toInt(-1);
    r.type.leftStickY = type["leftStickY"].toInt(-1);
    r.type.rightStickX = type["rightStickX"].toInt(-1);
    r.type.rightStickY = type["rightStickY"].toInt(-1);
    if (r.type.guid.empty())
    {
        error = QObject::tr("Missing device type");
        return false;
    }

    r.activePreset = json["activePreset"].toString().toStdString();

    for (QJsonValue const v : json["presets"].toArray())
    {
        Profile p;
        if (!profileFromJson(v.toObject(), p, error))
        {
            return false;
        }
        p.deviceGuid = r.type.guid;
        r.presets.push_back(p);
    }

    QJsonObject const instances = json["instancePresets"].toObject();
    for (auto it = instances.begin(); it != instances.end(); ++it)
    {
        r.instancePresets[it.key().toStdString()] = it.value().toString().toStdString();
    }

    record = r;
    return true;
}
