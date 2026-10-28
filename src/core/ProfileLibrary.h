#ifndef ODE2JOY_CORE_PROFILELIBRARY_H_
#define ODE2JOY_CORE_PROFILELIBRARY_H_

#include "Mapping.h"

#include <map>
#include <mutex>
#include <string>
#include <vector>

DeviceType deviceTypeOf(DeviceInfo const &device);

// One mapping per control: buttons to notes from 36, axes to CC 16 and up, hat X/Y to CC 80 and up.
// Axes 0/1 and 2/3 are declared as sticks when present.
Profile makeDefaultProfile(DeviceType const &type);

// Thread-safe store of everything known per joystick type (GUID).
class ProfileLibrary
{
public:
    // Adds an unknown type or refreshes name and counts of a known one. Returns true if the type is new.
    bool registerType(DeviceType const &type);

    std::vector<TypeRecord> records() const;
    bool getRecord(std::string const &guid, TypeRecord &record) const;
    // Replaces the record of record.type.guid, adding it if necessary.
    void setRecord(TypeRecord const &record);

    // A device-specific choice wins over the type's active preset; an empty name clears it.
    void setInstancePreset(std::string const &guid, std::string const &serial, std::string const &presetName);

    // Instance preset, then the type's active preset, then the first preset, then the built-in default.
    Profile resolve(DeviceInfo const &device) const;

    // Name of the preset resolve() picks; empty if the built-in default is used.
    std::string selectedPresetName(DeviceInfo const &device) const;

private:
    static Profile const *pick(TypeRecord const &record, std::string const &serial);

    mutable std::mutex m_mutex;
    std::map<std::string, TypeRecord> m_records;
};

#endif
