#ifndef PROFILESTORE_H_
#define PROFILESTORE_H_

#include "core/ProfileLibrary.h"

#include <QString>

// Persists the profiles of a ProfileLibrary as JSON files in the application config directory.
class ProfileStore
{
public:
    explicit ProfileStore(ProfileLibrary &library);

    void load();

    bool saveTypeProfile(Profile const &profile, QString &error);
    bool saveInstanceProfile(Profile const &profile, QString &error);
    void removeTypeProfile(std::string const &guid);
    void removeInstanceProfile(std::string const &guid, std::string const &serial);

    static bool readProfileFile(QString const &path, Profile &profile, QString &error);
    static bool writeProfileFile(QString const &path, Profile const &profile, QString &error);

private:
    QString directory() const;
    QString typePath(std::string const &guid) const;
    QString instancePath(std::string const &guid, std::string const &serial) const;

    ProfileLibrary &m_library;
};

#endif /* PROFILESTORE_H_ */
