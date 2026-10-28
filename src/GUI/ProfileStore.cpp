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

bool writeJson(QString const &path, QJsonObject const &object, QString &error)
{
    QDir().mkpath(QFileInfo(path).absolutePath());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        error = file.errorString();
        return false;
    }

    file.write(QJsonDocument(object).toJson());
    if (!file.commit())
    {
        error = file.errorString();
        return false;
    }

    return true;
}

bool readJson(QString const &path, QJsonObject &object, QString &error)
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

    object = doc.object();
    return true;
}

} // namespace

ProfileStore::ProfileStore(ProfileLibrary &library) :
    m_library(library)
{
}

QString ProfileStore::directory() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/types";
}

QString ProfileStore::pathFor(std::string const &guid) const
{
    // Device-provided strings must not be able to escape the config directory.
    QString const safe = QString::fromStdString(guid).replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
    return directory() + "/" + safe + ".json";
}

void ProfileStore::load()
{
    QDir dir(directory());
    for (QFileInfo const &file : dir.entryInfoList(QStringList() << "*.json", QDir::Files))
    {
        QJsonObject json;
        TypeRecord record;
        QString error;
        if (readJson(file.absoluteFilePath(), json, error) && recordFromJson(json, record, error))
        {
            m_library.setRecord(record);
        }
    }
}

bool ProfileStore::save(std::string const &guid, QString &error)
{
    TypeRecord record;
    if (!m_library.getRecord(guid, record))
    {
        error = QObject::tr("Unknown joystick type");
        return false;
    }

    return writeJson(pathFor(guid), recordToJson(record), error);
}

void ProfileStore::ensureSaved(std::string const &guid)
{
    if (!QFileInfo::exists(pathFor(guid)))
    {
        QString error;
        save(guid, error);
    }
}

bool ProfileStore::readProfileFile(QString const &path, Profile &profile, QString &error)
{
    QJsonObject json;
    return readJson(path, json, error) && profileFromJson(json, profile, error);
}

bool ProfileStore::writeProfileFile(QString const &path, Profile const &profile, QString &error)
{
    return writeJson(path, profileToJson(profile), error);
}
