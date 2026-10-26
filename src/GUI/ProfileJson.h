#ifndef PROFILEJSON_H_
#define PROFILEJSON_H_

#include "core/Mapping.h"

#include <QJsonObject>
#include <QString>

QJsonObject profileToJson(Profile const &profile);

// Returns false and sets error if the document is not a valid profile.
bool profileFromJson(QJsonObject const &json, Profile &profile, QString &error);

#endif /* PROFILEJSON_H_ */
