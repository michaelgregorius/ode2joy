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
#include "PipelineBridge.h"
#include "ProfileStore.h"

#include "core/Pipeline.h"
#include "core/SdlInput.h"

#include <QAction>
#include <QFileDialog>
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
    fileMenu->addAction(QMenu::tr("&Load profile for device type..."), this, &MainWindow::onActionLoadTypeProfile);
    fileMenu->addAction(QMenu::tr("Load profile for this &device only..."), this, &MainWindow::onActionLoadDeviceProfile);
    fileMenu->addAction(QMenu::tr("&Save profile of selected device..."), this, &MainWindow::onActionSaveProfile);
    fileMenu->addAction(QMenu::tr("&Reset to default profile"), this, &MainWindow::onActionResetProfile);
    fileMenu->addSeparator();
    QAction *quitAction = fileMenu->addAction(QMenu::tr("&Quit"), this, &MainWindow::onActionQuit);
    quitAction->setShortcuts(QKeySequence::Quit);
    mainWindowMenuBar->addMenu(fileMenu);

    QMenu *helpMenu = new QMenu(QMenu::tr("&Help"), mainWindowMenuBar);
    helpMenu->addAction(QMenu::tr("About Qt..."), this, &MainWindow::onActionAboutQt);
    mainWindowMenuBar->addMenu(helpMenu);

    connect(m_bridge.get(), &PipelineBridge::deviceConnected, this,
            [this](DeviceInfo info, QString profile) { m_deviceList->addDevice(info, profile); });
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

void MainWindow::onActionLoadTypeProfile()
{
    loadProfile(false);
}

void MainWindow::onActionLoadDeviceProfile()
{
    loadProfile(true);
}

void MainWindow::loadProfile(bool forThisDeviceOnly)
{
    DeviceInfo device;
    if (!m_deviceList->selectedDevice(device))
    {
        QMessageBox::information(this, tr("No joystick selected"), tr("Please select a joystick first."));
        return;
    }

    if (forThisDeviceOnly && device.serial.empty())
    {
        QMessageBox::information(this, tr("No serial number"),
                                 tr("This joystick does not report a serial number, so a profile can only be assigned to its type."));
        return;
    }

    QString const path = QFileDialog::getOpenFileName(this, tr("Load profile"), QString(), tr("Profiles (*.json)"));
    if (path.isEmpty())
    {
        return;
    }

    Profile profile;
    QString error;
    if (!ProfileStore::readProfileFile(path, profile, error))
    {
        QMessageBox::warning(this, tr("Cannot load profile"), error);
        return;
    }

    profile.deviceGuid = device.guid;
    profile.deviceSerial = forThisDeviceOnly ? device.serial : std::string();

    bool const saved = forThisDeviceOnly ? m_store->saveInstanceProfile(profile, error)
                                         : m_store->saveTypeProfile(profile, error);
    if (!saved)
    {
        QMessageBox::warning(this, tr("Cannot store profile"), error);
    }

    m_pipeline->reloadProfiles();
}

void MainWindow::onActionSaveProfile()
{
    DeviceInfo device;
    if (!m_deviceList->selectedDevice(device))
    {
        QMessageBox::information(this, tr("No joystick selected"), tr("Please select a joystick first."));
        return;
    }

    QString const path = QFileDialog::getSaveFileName(this, tr("Save profile"), QString(), tr("Profiles (*.json)"));
    if (path.isEmpty())
    {
        return;
    }

    QString error;
    if (!ProfileStore::writeProfileFile(path, m_library->resolve(device), error))
    {
        QMessageBox::warning(this, tr("Cannot save profile"), error);
    }
}

void MainWindow::onActionResetProfile()
{
    DeviceInfo device;
    if (!m_deviceList->selectedDevice(device))
    {
        QMessageBox::information(this, tr("No joystick selected"), tr("Please select a joystick first."));
        return;
    }

    if (!device.serial.empty())
    {
        m_store->removeInstanceProfile(device.guid, device.serial);
    }
    m_store->removeTypeProfile(device.guid);
    m_pipeline->reloadProfiles();
}

void MainWindow::onActionQuit()
{
    this->close();
}

void MainWindow::onActionAboutQt()
{
    QMessageBox::aboutQt(this);
}
