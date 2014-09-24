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

#include "JoystickDriver.h"

#include <QMenuBar>
#include <QMenu>
#include <QGraphicsPathItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent,
                       JoystickDriver *joystickDriver,
                       JoystickToMIDIMapper *joystickToMIDIMapper) :
    QMainWindow(parent),
    m_joystickDriver(joystickDriver),
    m_joystickToMIDIMapper(joystickToMIDIMapper)
{
    this->setWindowTitle("ode2joy");
    this->setWindowIcon(QIcon(":/images/application-icon.png"));

    // Build menu
    QMenuBar *mainWindowMenuBar = this->menuBar();
    QMenu *fileMenu = new QMenu(QMenu::tr("&File"), mainWindowMenuBar);

    QAction *quitAction = fileMenu->addAction(QMenu::tr("&Quit"));
    quitAction->setShortcuts(QKeySequence::Quit);
    mainWindowMenuBar->addMenu(fileMenu);

    QObject::connect(quitAction, SIGNAL(triggered()), SLOT(onActionQuit()));

    // Help menu
    QMenu *helpMenu = new QMenu(QMenu::tr("&Help"), mainWindowMenuBar);

    QAction *aboutQtAction = helpMenu->addAction(QMenu::tr("About Qt..."));
    mainWindowMenuBar->addMenu(helpMenu);

    QObject::connect(aboutQtAction, SIGNAL(triggered()), SLOT(onActionAboutQt()));

    if (m_joystickDriver)
    {
        m_joystickDriver->start(m_joystickToMIDIMapper);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    stopJoystickDriver();
    QMainWindow::closeEvent(event);
}

void MainWindow::onActionQuit()
{
    this->close();
}

void MainWindow::onActionAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::stopJoystickDriver() const
{
    if (m_joystickDriver)
    {
        m_joystickDriver->stop();
    }
}
