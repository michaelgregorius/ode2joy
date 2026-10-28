#ifndef PROFILEJSON_H_
#define PROFILEJSON_H_

#include "core/Mapping.h"

#include <QJsonObject>
#include <QString>

QJsonObject profileToJson(Profile const &profile);

// Returns false and sets error if the document is not a valid preset.
bool profileFromJson(QJsonObject const &json, Profile &profile, QString &error);

QJsonObject recordToJson(TypeRecord const &record);
bool recordFromJson(QJsonObject const &json, TypeRecord &record, QString &error);

#endif /* PROFILEJSON_H_ */
