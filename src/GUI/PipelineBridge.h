#ifndef PIPELINEBRIDGE_H_
#define PIPELINEBRIDGE_H_

#include "core/Pipeline.h"

#include <QObject>
#include <QString>

Q_DECLARE_METATYPE(DeviceInfo)

// Forwards pipeline callbacks (worker thread) as queued Qt signals (GUI thread).
class PipelineBridge : public QObject, public PipelineListener
{
    Q_OBJECT

public:
    explicit PipelineBridge(QObject *parent = nullptr);

    void onDeviceConnected(DeviceInfo const &info, std::string const &profileName) override;
    void onDeviceDisconnected(DeviceId id) override;
    void onDeviceProfileChanged(DeviceId id, std::string const &profileName) override;
    void onActivity(DeviceId id) override;
    void onError(std::string const &message) override;

signals:
    void deviceConnected(DeviceInfo info, QString profileName);
    void deviceDisconnected(quint32 id);
    void deviceProfileChanged(quint32 id, QString profileName);
    void activity(quint32 id);
    void error(QString message);
};

#endif /* PIPELINEBRIDGE_H_ */
