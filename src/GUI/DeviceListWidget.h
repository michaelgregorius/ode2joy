#ifndef DEVICELISTWIDGET_H_
#define DEVICELISTWIDGET_H_

#include "core/ControlEvent.h"

#include <QElapsedTimer>
#include <QHash>
#include <QTableWidget>
#include <QTimer>

class DeviceListWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit DeviceListWidget(QWidget *parent = nullptr);

    void addDevice(DeviceInfo const &info, QString const &profileName);
    void removeDevice(DeviceId id);
    void setProfileName(DeviceId id, QString const &profileName);
    void flashActivity(DeviceId id);

    bool selectedDevice(DeviceInfo &info) const;

signals:
    void deviceEnabledChanged(quint32 id, bool enabled);

private slots:
    void onItemChanged(QTableWidgetItem *item);
    void updateActivity();

private:
    enum Column { ColEnabled, ColName, ColProfile, ColActivity, ColCount };

    int rowOf(DeviceId id) const;

    QHash<DeviceId, DeviceInfo> m_devices;
    QHash<DeviceId, qint64> m_lastActivity;
    QElapsedTimer m_clock;
    QTimer m_timer;
};

#endif /* DEVICELISTWIDGET_H_ */
