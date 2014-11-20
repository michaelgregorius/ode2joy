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

#include "SessionWidget.h"

#include "Session.h"

#include "Drivers/Joystick/JoystickDriverFactory.h"
#include "Drivers/MIDI/MIDIDriverFactory.h"

#include <QMenuBar>
#include <QMenu>
#include <QGraphicsPathItem>
#include <QMessageBox>

#include <iostream>

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

    // Drivers menu
    QMenu *driverMenu = new QMenu(QMenu::tr("&Driver"), mainWindowMenuBar);

    // Joystick sub menu
    QMenu *joystickDriverMenu = new QMenu(QMenu::tr("&Joystick"), driverMenu);
    driverMenu->addMenu(joystickDriverMenu);
    JoystickDriverFactory const *factory = JoystickDriverFactory::getInstance();
    JoystickDriverFactory::DriverDescriptionCollection descriptions = factory->getDriverDescriptions();

    for (auto d : descriptions)
    {
        QAction *joysticDriverAction = joystickDriverMenu->addAction(QString::fromStdString(d.getDescription()));
        joysticDriverAction->setData(QVariant(QString::fromStdString(d.getKey())));
        QObject::connect(joysticDriverAction, SIGNAL(triggered()), SLOT(onActionJoystickDriverSelected()));
    }

    // MIDI sub menu
    QMenu *midiDriverMenu = new QMenu(QMenu::tr("&MIDI"), driverMenu);
    driverMenu->addMenu(midiDriverMenu);
    MIDIDriverFactory const *midiFactory = MIDIDriverFactory::getInstance();
    MIDIDriverFactory::DriverDescriptionCollection midiDescriptions = midiFactory->getDriverDescriptions();

    for (auto d : midiDescriptions)
    {
        QAction *midiDriverAction = midiDriverMenu->addAction(QString::fromStdString(d.getDescription()));
        midiDriverAction->setData(QVariant(QString::fromStdString(d.getKey())));
        QObject::connect(midiDriverAction, SIGNAL(triggered()), SLOT(onActionMIDIDriverSelected()));
    }

    mainWindowMenuBar->addMenu(driverMenu);

    // Help menu
    QMenu *helpMenu = new QMenu(QMenu::tr("&Help"), mainWindowMenuBar);

    QAction *aboutQtAction = helpMenu->addAction(QMenu::tr("About Qt..."));
    mainWindowMenuBar->addMenu(helpMenu);

    QObject::connect(aboutQtAction, SIGNAL(triggered()), SLOT(onActionAboutQt()));

    m_sessionWidget = new SessionWidget(this, m_session);
    setCentralWidget(m_sessionWidget);
}

MainWindow::~MainWindow()
{
    m_sessionWidget->setSession(0);

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

    m_sessionWidget->setSession(m_session);
}

void MainWindow::onActionQuit()
{
    this->close();
}

void MainWindow::onActionJoystickDriverSelected()
{
    if (!m_session)
    {
        QMessageBox::information(this,
                                 tr("No session started"),
                                 tr("Please start a session before selecting a joystick driver!"));

        return;
    }

    QObject *s = sender();

    QAction *action = dynamic_cast<QAction*>(s);
    if (action)
    {
        std::string key = action->data().toString().toStdString();

        JoystickDriver *driver = JoystickDriverFactory::getInstance()->createDriver(key);
        m_session->setJoystickDriver(driver);
    }
}

void MainWindow::onActionMIDIDriverSelected()
{
    if (!m_session)
    {
        QMessageBox::information(this,
                                 tr("No session started"),
                                 tr("Please start a session before selecting a MIDI driver!"));

        return;
    }

    QObject *s = sender();

    QAction *action = dynamic_cast<QAction*>(s);
    if (action)
    {
        std::string key = action->data().toString().toStdString();

        MIDIDriver *driver = MIDIDriverFactory::getInstance()->createDriver(key);
        m_session->setMIDIDriver(driver);
    }
}

void MainWindow::onActionAboutQt()
{
    QMessageBox::aboutQt(this);
}
