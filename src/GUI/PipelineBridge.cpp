#include "PipelineBridge.h"

PipelineBridge::PipelineBridge(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<DeviceInfo>("DeviceInfo");
}

void PipelineBridge::onDeviceConnected(DeviceInfo const &info, std::string const &profileName)
{
    emit deviceConnected(info, QString::fromStdString(profileName));
}

void PipelineBridge::onDeviceDisconnected(DeviceId id)
{
    emit deviceDisconnected(id);
}

void PipelineBridge::onDeviceProfileChanged(DeviceId id, std::string const &profileName)
{
    emit deviceProfileChanged(id, QString::fromStdString(profileName));
}

void PipelineBridge::onActivity(DeviceId id)
{
    emit activity(id);
}

void PipelineBridge::onControlActivity(DeviceInfo const &info, ControlEvent const &event)
{
    emit controlActivity(QString::fromStdString(info.guid), static_cast<int>(event.type), event.index, event.value);
}

void PipelineBridge::onError(std::string const &message)
{
    emit error(QString::fromStdString(message));
}
