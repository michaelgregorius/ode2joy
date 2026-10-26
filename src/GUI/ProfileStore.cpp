#include "ProfileStore.h"

#include "ProfileJson.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>

namespace
{

// Device-provided strings must not be able to escape the profile directory.
QString sanitize(std::string const &s)
{
    return QString::fromStdString(s).replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
}

} // namespace

ProfileStore::ProfileStore(ProfileLibrary &library) :
    m_library(library)
{
}

QString ProfileStore::directory() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/profiles";
}

QString ProfileStore::typePath(std::string const &guid) const
{
    return directory() + "/type-" + sanitize(guid) + ".json";
}

QString ProfileStore::instancePath(std::string const &guid, std::string const &serial) const
{
    return directory() + "/device-" + sanitize(guid) + "-" + sanitize(serial) + ".json";
}

void ProfileStore::load()
{
    QDir dir(directory());
    for (QFileInfo const &file : dir.entryInfoList(QStringList() << "*.json", QDir::Files))
    {
        Profile p;
        QString error;
        if (!readProfileFile(file.absoluteFilePath(), p, error) || p.deviceGuid.empty())
        {
            continue;
        }

        if (p.deviceSerial.empty())
        {
            m_library.setTypeProfile(p.deviceGuid, p);
        }
        else
        {
            m_library.setInstanceProfile(p.deviceGuid, p.deviceSerial, p);
        }
    }
}

bool ProfileStore::saveTypeProfile(Profile const &profile, QString &error)
{
    Profile p = profile;
    p.deviceSerial.clear();
    m_library.setTypeProfile(p.deviceGuid, p);
    return writeProfileFile(typePath(p.deviceGuid), p, error);
}

bool ProfileStore::saveInstanceProfile(Profile const &profile, QString &error)
{
    m_library.setInstanceProfile(profile.deviceGuid, profile.deviceSerial, profile);
    return writeProfileFile(instancePath(profile.deviceGuid, profile.deviceSerial), profile, error);
}

void ProfileStore::removeTypeProfile(std::string const &guid)
{
    m_library.clearTypeProfile(guid);
    QFile::remove(typePath(guid));
}

void ProfileStore::removeInstanceProfile(std::string const &guid, std::string const &serial)
{
    m_library.clearInstanceProfile(guid, serial);
    QFile::remove(instancePath(guid, serial));
}

bool ProfileStore::readProfileFile(QString const &path, Profile &profile, QString &error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        error = file.errorString();
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument const doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (doc.isNull() || !doc.isObject())
    {
        error = parseError.errorString();
        return false;
    }

    return profileFromJson(doc.object(), profile, error);
}

bool ProfileStore::writeProfileFile(QString const &path, Profile const &profile, QString &error)
{
    QDir().mkpath(QFileInfo(path).absolutePath());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        error = file.errorString();
        return false;
    }

    file.write(QJsonDocument(profileToJson(profile)).toJson());
    if (!file.commit())
    {
        error = file.errorString();
        return false;
    }

    return true;
}
