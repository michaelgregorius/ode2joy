#include "DeviceListWidget.h"

#include <QBrush>
#include <QHeaderView>

namespace
{

int const flashDurationMs = 150;

} // namespace

DeviceListWidget::DeviceListWidget(QWidget *parent) :
    QTableWidget(0, ColCount, parent)
{
    setHorizontalHeaderLabels(QStringList() << tr("Enabled") << tr("Joystick") << tr("Profile") << tr("Activity"));
    horizontalHeader()->setSectionResizeMode(ColName, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(ColProfile, QHeaderView::ResizeToContents);
    verticalHeader()->hide();
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    m_clock.start();
    m_timer.setInterval(50);
    connect(&m_timer, &QTimer::timeout, this, &DeviceListWidget::updateActivity);
    connect(this, &QTableWidget::itemChanged, this, &DeviceListWidget::onItemChanged);
}

int DeviceListWidget::rowOf(DeviceId id) const
{
    for (int row = 0; row < rowCount(); ++row)
    {
        QTableWidgetItem const *item = this->item(row, ColName);
        if (item && item->data(Qt::UserRole).toUInt() == id)
        {
            return row;
        }
    }
    return -1;
}

void DeviceListWidget::addDevice(DeviceInfo const &info, QString const &profileName)
{
    if (rowOf(info.id) >= 0)
    {
        return;
    }

    QString title = QString::fromStdString(info.name);
    if (info.instance > 1)
    {
        title += QString(" #%1").arg(info.instance);
    }
    title += tr(" (%1 axes, %2 buttons, %3 hats)").arg(info.numAxes).arg(info.numButtons).arg(info.numHats);

    QSignalBlocker blocker(this);
    int const row = rowCount();
    insertRow(row);

    QTableWidgetItem *enabled = new QTableWidgetItem();
    enabled->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    enabled->setCheckState(Qt::Checked);
    setItem(row, ColEnabled, enabled);

    QTableWidgetItem *name = new QTableWidgetItem(title);
    name->setData(Qt::UserRole, info.id);
    name->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    setItem(row, ColName, name);

    QTableWidgetItem *profile = new QTableWidgetItem(profileName);
    profile->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    setItem(row, ColProfile, profile);

    QTableWidgetItem *activity = new QTableWidgetItem();
    activity->setFlags(Qt::ItemIsEnabled);
    setItem(row, ColActivity, activity);

    m_devices.insert(info.id, info);
}

void DeviceListWidget::removeDevice(DeviceId id)
{
    int const row = rowOf(id);
    if (row >= 0)
    {
        removeRow(row);
    }
    m_devices.remove(id);
    m_lastActivity.remove(id);
}

void DeviceListWidget::setProfileName(DeviceId id, QString const &profileName)
{
    int const row = rowOf(id);
    if (row >= 0)
    {
        item(row, ColProfile)->setText(profileName);
    }
}

void DeviceListWidget::flashActivity(DeviceId id)
{
    int const row = rowOf(id);
    if (row < 0)
    {
        return;
    }

    m_lastActivity[id] = m_clock.elapsed();
    item(row, ColActivity)->setBackground(QBrush(Qt::green));
    if (!m_timer.isActive())
    {
        m_timer.start();
    }
}

void DeviceListWidget::updateActivity()
{
    qint64 const now = m_clock.elapsed();
    bool anyActive = false;

    for (auto it = m_lastActivity.begin(); it != m_lastActivity.end();)
    {
        if (now - it.value() >= flashDurationMs)
        {
            int const row = rowOf(it.key());
            if (row >= 0)
            {
                item(row, ColActivity)->setBackground(QBrush());
            }
            it = m_lastActivity.erase(it);
        }
        else
        {
            anyActive = true;
            ++it;
        }
    }

    if (!anyActive)
    {
        m_timer.stop();
    }
}

bool DeviceListWidget::selectedDevice(DeviceInfo &info) const
{
    int const row = currentRow();
    if (row < 0)
    {
        return false;
    }

    DeviceId const id = item(row, ColName)->data(Qt::UserRole).toUInt();
    auto it = m_devices.find(id);
    if (it == m_devices.end())
    {
        return false;
    }

    info = it.value();
    return true;
}

void DeviceListWidget::onItemChanged(QTableWidgetItem *changed)
{
    if (changed->column() != ColEnabled)
    {
        return;
    }

    QTableWidgetItem const *name = item(changed->row(), ColName);
    emit deviceEnabledChanged(name->data(Qt::UserRole).toUInt(), changed->checkState() == Qt::Checked);
}
