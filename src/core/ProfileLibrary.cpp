#include "ProfileLibrary.h"

#include <algorithm>

DeviceType deviceTypeOf(DeviceInfo const &device)
{
    DeviceType t;
    t.guid = device.guid;
    t.name = device.name;
    t.numAxes = device.numAxes;
    t.numButtons = device.numButtons;
    t.numHats = device.numHats;
    t.leftStickX = device.leftStickX;
    t.leftStickY = device.leftStickY;
    t.rightStickX = device.rightStickX;
    t.rightStickY = device.rightStickY;
    return t;
}

Profile makeDefaultProfile(DeviceType const &type)
{
    Profile p;
    p.name = "Default";
    p.deviceGuid = type.guid;

    for (int i = 0; i < type.numButtons; ++i)
    {
        Mapping m;
        m.source.type = ControlType::Button;
        m.source.index = i;
        m.target.kind = MidiTargetKind::Note;
        m.target.number = static_cast<uint8_t>(std::min(36 + i, 127));
        m.target.onValue = 100;
        p.mappings.push_back(m);
    }

    for (int i = 0; i < type.numAxes; ++i)
    {
        Mapping m;
        m.source.type = ControlType::Axis;
        m.source.index = i;
        m.target.kind = MidiTargetKind::ControlChange;
        m.target.number = static_cast<uint8_t>(std::min(16 + i, 127));
        m.deadZone = 0.05f;
        p.mappings.push_back(m);
    }

    for (int i = 0; i < type.numHats; ++i)
    {
        for (int c = 0; c < 2; ++c)
        {
            Mapping m;
            m.source.type = ControlType::Hat;
            m.source.index = i;
            m.hatComponent = c == 0 ? HatComponent::X : HatComponent::Y;
            m.target.kind = MidiTargetKind::ControlChange;
            m.target.number = static_cast<uint8_t>(std::min(80 + 2 * i + c, 127));
            p.mappings.push_back(m);
        }
    }

    // Without gamepad information, assume axes 0/1 and 2/3 are the sticks.
    auto const pick = [&type](int value, int fallback) {
        return value >= 0 && value < type.numAxes ? value : fallback;
    };
    bool const known = type.leftStickX >= 0 || type.rightStickX >= 0;

    if (known ? (type.leftStickX >= 0 && type.leftStickY >= 0) : type.numAxes >= 2)
    {
        Stick left;
        left.label = "Left stick";
        left.xAxis = pick(type.leftStickX, 0);
        left.yAxis = pick(type.leftStickY, 1);
        p.sticks.push_back(left);
    }
    if (known ? (type.rightStickX >= 0 && type.rightStickY >= 0) : type.numAxes >= 4)
    {
        Stick right;
        right.label = "Right stick";
        right.xAxis = pick(type.rightStickX, 2);
        right.yAxis = pick(type.rightStickY, 3);
        p.sticks.push_back(right);
    }

    return p;
}

bool ProfileLibrary::registerType(DeviceType const &type)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_records.find(type.guid);
    if (it == m_records.end())
    {
        TypeRecord r;
        r.type = type;
        m_records[type.guid] = r;
        return true;
    }

    it->second.type = type;
    return false;
}

std::vector<TypeRecord> ProfileLibrary::records() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<TypeRecord> result;
    for (auto const &entry : m_records)
    {
        result.push_back(entry.second);
    }
    return result;
}

bool ProfileLibrary::getRecord(std::string const &guid, TypeRecord &record) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_records.find(guid);
    if (it == m_records.end())
    {
        return false;
    }

    record = it->second;
    return true;
}

void ProfileLibrary::setRecord(TypeRecord const &record)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_records[record.type.guid] = record;
}

void ProfileLibrary::setInstancePreset(std::string const &guid, std::string const &serial, std::string const &presetName)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_records.find(guid);
    if (it == m_records.end() || serial.empty())
    {
        return;
    }

    if (presetName.empty())
    {
        it->second.instancePresets.erase(serial);
    }
    else
    {
        it->second.instancePresets[serial] = presetName;
    }
}

Profile const *ProfileLibrary::pick(TypeRecord const &record, std::string const &serial)
{
    if (record.presets.empty())
    {
        return nullptr;
    }

    std::string wanted = record.activePreset;
    auto inst = record.instancePresets.find(serial);
    if (!serial.empty() && inst != record.instancePresets.end())
    {
        wanted = inst->second;
    }

    for (Profile const &p : record.presets)
    {
        if (p.name == wanted)
        {
            return &p;
        }
    }

    return &record.presets.front();
}

Profile ProfileLibrary::resolve(DeviceInfo const &device) const
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_records.find(device.guid);
        if (it != m_records.end())
        {
            Profile const *p = pick(it->second, device.serial);
            if (p)
            {
                return *p;
            }
        }
    }

    return makeDefaultProfile(deviceTypeOf(device));
}

std::string ProfileLibrary::selectedPresetName(DeviceInfo const &device) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_records.find(device.guid);
    if (it == m_records.end())
    {
        return std::string();
    }

    Profile const *p = pick(it->second, device.serial);
    return p ? p->name : std::string();
}
