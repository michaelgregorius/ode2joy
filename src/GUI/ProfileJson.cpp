#include "ProfileJson.h"

#include <QJsonArray>

#include <algorithm>
#include <cstddef>

namespace
{

int const profileVersion = 1;

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
    { MidiTargetKind::ChannelAftertouch, "aftertouch" } };
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

    QJsonObject root;
    root["version"] = profileVersion;
    root["name"] = QString::fromStdString(profile.name);
    root["deviceGuid"] = QString::fromStdString(profile.deviceGuid);
    root["deviceSerial"] = QString::fromStdString(profile.deviceSerial);
    root["mappings"] = mappings;
    return root;
}

bool profileFromJson(QJsonObject const &json, Profile &profile, QString &error)
{
    if (json["version"].toInt() != profileVersion)
    {
        error = QObject::tr("Unsupported profile version");
        return false;
    }

    Profile p;
    p.name = json["name"].toString().toStdString();
    p.deviceGuid = json["deviceGuid"].toString().toStdString();
    p.deviceSerial = json["deviceSerial"].toString().toStdString();

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

        m.source.index = source["index"].toInt();
        if (m.source.type == ControlType::Hat
            && !fromName(hatComponents, source["component"].toString(), m.hatComponent))
        {
            error = QObject::tr("Invalid hat component in mapping %1").arg(i + 1);
            return false;
        }

        m.target.channel = static_cast<uint8_t>(std::max(0, std::min(15, target["channel"].toInt(1) - 1)));
        m.target.number = clampByte(target["number"].toInt());
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

    profile = p;
    return true;
}
