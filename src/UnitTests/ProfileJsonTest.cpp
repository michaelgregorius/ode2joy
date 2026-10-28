#include "GUI/ProfileJson.h"

#include "core/ProfileLibrary.h"

#include <QJsonDocument>

#include <iostream>

namespace
{

int const error = 1;

bool sameMapping(Mapping const &a, Mapping const &b)
{
    return a.source.type == b.source.type && a.source.index == b.source.index
        && a.hatComponent == b.hatComponent && a.target.kind == b.target.kind
        && a.target.channel == b.target.channel && a.target.number == b.target.number
        && a.target.onValue == b.target.onValue && a.target.offValue == b.target.offValue
        && a.range == b.range && a.curve.type == b.curve.type && a.curve.amount == b.curve.amount
        && a.deadZone == b.deadZone && a.smoothing == b.smoothing && a.invert == b.invert
        && a.maxRateHz == b.maxRateHz;
}

TypeRecord sampleRecord()
{
    DeviceType type;
    type.guid = "03000000deadbeef";
    type.name = "Pad";
    type.numAxes = 4;
    type.numButtons = 8;
    type.numHats = 1;

    TypeRecord r;
    r.type = type;

    Profile a = makeDefaultProfile(type);
    a.mappings[8].target.channel = 5;
    a.mappings[8].invert = true;
    a.mappings[8].curve = { CurveType::Logarithmic, 4.5f };
    a.mappings[8].range = AxisRange::Unipolar;
    a.sticks[0].radialDeadZone = 0.2f;
    a.sticks[0].label = "Left";
    ControlLabel label;
    label.type = ControlType::Button;
    label.index = 3;
    label.text = "Kick";
    a.labels.push_back(label);

    Profile b = makeDefaultProfile(type);
    b.name = "Second";

    r.presets = { a, b };
    r.activePreset = "Second";
    r.instancePresets["serial-1"] = "Default";
    return r;
}

int testRoundTrip()
{
    TypeRecord const original = sampleRecord();

    QString err;
    QJsonDocument const doc(recordToJson(original));
    QJsonDocument const parsed = QJsonDocument::fromJson(doc.toJson());

    TypeRecord loaded;
    if (!recordFromJson(parsed.object(), loaded, err))
    {
        return error;
    }

    if (loaded.type.guid != original.type.guid || loaded.type.numHats != 1 || loaded.activePreset != "Second"
        || loaded.presets.size() != 2 || loaded.instancePresets != original.instancePresets)
    {
        return error;
    }

    Profile const &a = loaded.presets[0];
    Profile const &oa = original.presets[0];
    if (a.mappings.size() != oa.mappings.size() || a.sticks.size() != 2 || a.labels.size() != 1
        || a.sticks[0].label != "Left" || a.sticks[0].radialDeadZone != 0.2f || a.labels[0].text != "Kick")
    {
        return error;
    }

    for (size_t i = 0; i < a.mappings.size(); ++i)
    {
        if (!sameMapping(a.mappings[i], oa.mappings[i]))
        {
            return error;
        }
    }

    return 0;
}

int testRejectsInvalid()
{
    QString err;
    Profile p;
    QJsonObject bad;
    bad["version"] = 99;
    if (profileFromJson(bad, p, err))
    {
        return error;
    }

    TypeRecord r;
    QJsonObject noType;
    noType["version"] = 2;
    return recordFromJson(noType, r, err) ? error : 0;
}

} // namespace

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 0;
    }

    std::string const name = argv[1];
    if (name == "test_json_round_trip")
    {
        return testRoundTrip();
    }
    if (name == "test_json_rejects_invalid")
    {
        return testRejectsInvalid();
    }

    std::cerr << "Unknown test " << name << std::endl;
    return error;
}
