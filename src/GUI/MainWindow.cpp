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

#include "Session.h"

// TODO Fetch from factories when selected in the GUI
#include "Drivers/Joystick/JoystickDriverLinux.h"
#include "MIDIInterfaceALSA.h"

#include <QMenuBar>
#include <QMenu>
#include <QGraphicsPathItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_session(0)
{
    this->setWindowTitle("ode2joy");
    this->setWindowIcon(QIcon(":/images/application-icon.png"));

    // Build menu
    QMenuBar *mainWindowMenuBar = this->menuBar();
    QMenu *fileMenu = new QMenu(QMenu::tr("&File"), mainWindowMenuBar);

    QAction *newSessionAction = fileMenu->addAction(QMenu::tr("&New session"));
    newSessionAction->setShortcuts(QKeySequence::New);
    QObject::connect(newSessionAction, SIGNAL(triggered()), SLOT(onActionNewSession()));

    QAction *quitAction = fileMenu->addAction(QMenu::tr("&Quit"));
    quitAction->setShortcuts(QKeySequence::Quit);
    QObject::connect(quitAction, SIGNAL(triggered()), SLOT(onActionQuit()));

    mainWindowMenuBar->addMenu(fileMenu);

    // Help menu
    QMenu *helpMenu = new QMenu(QMenu::tr("&Help"), mainWindowMenuBar);

    QAction *aboutQtAction = helpMenu->addAction(QMenu::tr("About Qt..."));
    mainWindowMenuBar->addMenu(helpMenu);

    QObject::connect(aboutQtAction, SIGNAL(triggered()), SLOT(onActionAboutQt()));
}

MainWindow::~MainWindow()
{
    if (m_session)
    {
        delete m_session;
        m_session = 0;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}

void MainWindow::onActionNewSession()
{
    if (m_session)
    {
        delete m_session;
    }

    m_session = new Session();

    // Create a joystick
    JoystickDriver *joystickDriver = new JoystickDriverLinux();
    m_session->setJoystickDriver(joystickDriver);

    // Create MIDI interface
    MIDIInterface *midiInterface = new MIDIInterfaceALSA();
    m_session->setMIDIInterface(midiInterface);
}

void MainWindow::onActionQuit()
{
    this->close();
}

void MainWindow::onActionAboutQt()
{
    QMessageBox::aboutQt(this);
}
