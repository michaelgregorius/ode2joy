#include "ProfileLibrary.h"

#include <algorithm>

Profile makeDefaultProfile(DeviceInfo const &device)
{
    Profile p;
    p.name = "Default";
    p.deviceGuid = device.guid;

    for (int i = 0; i < device.numButtons; ++i)
    {
        Mapping m;
        m.source.type = ControlType::Button;
        m.source.index = i;
        m.target.kind = MidiTargetKind::Note;
        m.target.number = static_cast<uint8_t>(std::min(36 + i, 127));
        m.target.onValue = 100;
        p.mappings.push_back(m);
    }

    for (int i = 0; i < device.numAxes; ++i)
    {
        Mapping m;
        m.source.type = ControlType::Axis;
        m.source.index = i;
        m.target.kind = MidiTargetKind::ControlChange;
        m.target.number = static_cast<uint8_t>(std::min(16 + i, 127));
        m.deadZone = 0.05f;
        p.mappings.push_back(m);
    }

    for (int i = 0; i < device.numHats; ++i)
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

    return p;
}

void ProfileLibrary::setTypeProfile(std::string const &guid, Profile const &profile)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_typeProfiles[guid] = profile;
}

void ProfileLibrary::setInstanceProfile(std::string const &guid, std::string const &serial, Profile const &profile)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_instanceProfiles[std::make_pair(guid, serial)] = profile;
}

void ProfileLibrary::clearTypeProfile(std::string const &guid)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_typeProfiles.erase(guid);
}

void ProfileLibrary::clearInstanceProfile(std::string const &guid, std::string const &serial)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_instanceProfiles.erase(std::make_pair(guid, serial));
}

Profile ProfileLibrary::resolve(DeviceInfo const &device) const
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!device.serial.empty())
        {
            auto it = m_instanceProfiles.find(std::make_pair(device.guid, device.serial));
            if (it != m_instanceProfiles.end())
            {
                return it->second;
            }
        }

        auto it = m_typeProfiles.find(device.guid);
        if (it != m_typeProfiles.end())
        {
            return it->second;
        }
    }

    return makeDefaultProfile(device);
}
