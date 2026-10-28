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

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>

#include <memory>

class DeviceListWidget;
class Pipeline;
class PipelineBridge;
class ProfileLibrary;
class ProfileStore;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent);
    virtual ~MainWindow();

private slots:
    void onActionEditMappings();
    void onActionQuit();
    void onActionAboutQt();
    void onPresetSelected(quint32 id, QString const &name);

private:
    void refreshPresetChoices();

private:
    std::shared_ptr<ProfileLibrary> m_library;
    std::unique_ptr<ProfileStore> m_store;
    std::unique_ptr<PipelineBridge> m_bridge;
    std::unique_ptr<Pipeline> m_pipeline;
    DeviceListWidget *m_deviceList;
};

#endif /* MAINWINDOW_H_ */
