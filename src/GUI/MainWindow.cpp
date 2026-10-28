/*
 * Copyright (C) 2014 Michael Gregorius
 *
 * This file is part of ode2joy.
 *
 * ode2joy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ode2joy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ode2joy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MainWindow.h"

#include "DeviceListWidget.h"
#include "MappingEditorDialog.h"
#include "PipelineBridge.h"
#include "ProfileStore.h"

#include "core/Pipeline.h"
#include "core/SdlInput.h"

#include <QAction>
#include <QIcon>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_library(std::make_shared<ProfileLibrary>()),
    m_store(new ProfileStore(*m_library)),
    m_bridge(new PipelineBridge()),
    m_deviceList(new DeviceListWidget(this))
{
    this->setWindowTitle("ode2joy");
    this->setWindowIcon(QIcon(":/images/application-icon.png"));
    resize(640, 320);
    setCentralWidget(m_deviceList);

    QMenuBar *mainWindowMenuBar = this->menuBar();
    QMenu *fileMenu = new QMenu(QMenu::tr("&File"), mainWindowMenuBar);
    QAction *quitAction = fileMenu->addAction(QMenu::tr("&Quit"), this, &MainWindow::onActionQuit);
    quitAction->setShortcuts(QKeySequence::Quit);
    mainWindowMenuBar->addMenu(fileMenu);

    QMenu *editMenu = new QMenu(QMenu::tr("&Edit"), mainWindowMenuBar);
    QAction *editMappingsAction = editMenu->addAction(QMenu::tr("&Mapping presets..."), this, &MainWindow::onActionEditMappings);
    editMappingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    mainWindowMenuBar->addMenu(editMenu);

    QMenu *helpMenu = new QMenu(QMenu::tr("&Help"), mainWindowMenuBar);
    helpMenu->addAction(QMenu::tr("About Qt..."), this, &MainWindow::onActionAboutQt);
    mainWindowMenuBar->addMenu(helpMenu);

    connect(m_bridge.get(), &PipelineBridge::deviceConnected, this,
            [this](DeviceInfo info, QString profile) {
                m_deviceList->addDevice(info, profile);
                m_store->ensureSaved(info.guid);
                refreshPresetChoices();
            });
    connect(m_bridge.get(), &PipelineBridge::deviceDisconnected, this,
            [this](quint32 id) { m_deviceList->removeDevice(id); });
    connect(m_bridge.get(), &PipelineBridge::deviceProfileChanged, this,
            [this](quint32 id, QString profile) { m_deviceList->setProfileName(id, profile); });
    connect(m_bridge.get(), &PipelineBridge::activity, this,
            [this](quint32 id) { m_deviceList->flashActivity(id); });
    connect(m_bridge.get(), &PipelineBridge::error, this,
            [this](QString message) { statusBar()->showMessage(message, 10000); });

    m_store->load();

    m_pipeline.reset(new Pipeline(std::unique_ptr<InputBackend>(new SdlInput()), createRtMidiOutput, m_library));
    m_pipeline->setListener(m_bridge.get());

    connect(m_deviceList, &DeviceListWidget::deviceEnabledChanged, this,
            [this](quint32 id, bool enabled) { m_pipeline->setDeviceEnabled(id, enabled); });
    connect(m_deviceList, &DeviceListWidget::presetSelected, this, &MainWindow::onPresetSelected);

    std::string error;
    if (!m_pipeline->start(error))
    {
        QMessageBox::critical(this, tr("Joystick input unavailable"),
                              tr("Could not initialize SDL: %1").arg(QString::fromStdString(error)));
    }
}

MainWindow::~MainWindow()
{
    // The worker thread must stop before the bridge and library go away.
    m_pipeline->stop();
}

void MainWindow::refreshPresetChoices()
{
    for (DeviceInfo const &device : m_deviceList->devices())
    {
        TypeRecord record;
        QStringList names;
        if (m_library->getRecord(device.guid, record))
        {
            for (Profile const &p : record.presets)
            {
                names << QString::fromStdString(p.name);
            }
        }

        QString current = QString::fromStdString(m_library->selectedPresetName(device));
        if (current.isEmpty())
        {
            current = tr("Default");
            names.clear();
            names << current;
        }

        m_deviceList->setPresets(device.id, names, current);
    }
}

void MainWindow::onPresetSelected(quint32 id, QString const &name)
{
    DeviceInfo device;
    TypeRecord record;
    if (!m_deviceList->device(id, device) || !m_library->getRecord(device.guid, record) || record.presets.empty())
    {
        return;
    }

    // Devices that report a serial number get their own choice; others share the type's active preset.
    if (!device.serial.empty())
    {
        m_library->setInstancePreset(device.guid, device.serial, name.toStdString());
    }
    else
    {
        record.activePreset = name.toStdString();
        m_library->setRecord(record);
    }

    QString error;
    if (!m_store->save(device.guid, error))
    {
        statusBar()->showMessage(error, 10000);
    }

    m_pipeline->reloadProfiles();
}

void MainWindow::onActionEditMappings()
{
    DeviceInfo device;
    std::string const guid = m_deviceList->selectedDevice(device) ? device.guid : std::string();

    MappingEditorDialog dialog(m_library, *m_store, [this]() { m_pipeline->reloadProfiles(); }, guid, this);
    dialog.exec();

    refreshPresetChoices();
}

void MainWindow::onActionQuit()
{
    this->close();
}

void MainWindow::onActionAboutQt()
{
    QMessageBox::aboutQt(this);
}
