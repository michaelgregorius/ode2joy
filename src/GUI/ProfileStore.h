#ifndef PROFILESTORE_H_
#define PROFILESTORE_H_

#include "core/ProfileLibrary.h"

#include <QString>

// Persists the type records of a ProfileLibrary as one JSON file each in the application config directory.
class ProfileStore
{
public:
    explicit ProfileStore(ProfileLibrary &library);

    void load();

    // Writes the library's record of the type; returns false and sets error on failure.
    bool save(std::string const &guid, QString &error);
    // Writes the record only if no file exists yet.
    void ensureSaved(std::string const &guid);

    static bool readProfileFile(QString const &path, Profile &profile, QString &error);
    static bool writeProfileFile(QString const &path, Profile const &profile, QString &error);

private:
    QString directory() const;
    QString pathFor(std::string const &guid) const;

    ProfileLibrary &m_library;
};

#endif /* PROFILESTORE_H_ */
