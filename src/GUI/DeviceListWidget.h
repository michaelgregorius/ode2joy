#ifndef DEVICELISTWIDGET_H_
#define DEVICELISTWIDGET_H_

#include "core/ControlEvent.h"

#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QTimer>

class DeviceListWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit DeviceListWidget(QWidget *parent = nullptr);

    void addDevice(DeviceInfo const &info, QString const &presetName);
    void removeDevice(DeviceId id);
    // Shows presetName as the current choice, adding it to the list if needed.
    void setProfileName(DeviceId id, QString const &presetName);
    void setPresets(DeviceId id, QStringList const &names, QString const &current);
    void flashActivity(DeviceId id);

    bool selectedDevice(DeviceInfo &info) const;
    QList<DeviceInfo> devices() const;
    bool device(DeviceId id, DeviceInfo &info) const;

signals:
    void deviceEnabledChanged(quint32 id, bool enabled);
    // The user picked a preset in the device's combo box.
    void presetSelected(quint32 id, QString name);

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
