#ifndef ODE2JOY_CORE_PROFILELIBRARY_H_
#define ODE2JOY_CORE_PROFILELIBRARY_H_

#include "Mapping.h"

#include <map>
#include <mutex>
#include <string>

Profile makeDefaultProfile(DeviceInfo const &device);

// Thread-safe store of profiles per device type (GUID) and per physical device (GUID + serial).
class ProfileLibrary
{
public:
    void setTypeProfile(std::string const &guid, Profile const &profile);
    void setInstanceProfile(std::string const &guid, std::string const &serial, Profile const &profile);
    void clearTypeProfile(std::string const &guid);
    void clearInstanceProfile(std::string const &guid, std::string const &serial);

    // Instance profile, then type profile, then the built-in default.
    Profile resolve(DeviceInfo const &device) const;

private:
    mutable std::mutex m_mutex;
    std::map<std::string, Profile> m_typeProfiles;
    std::map<std::pair<std::string, std::string>, Profile> m_instanceProfiles;
};

#endif
