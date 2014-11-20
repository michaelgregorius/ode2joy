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

#include "SessionWidget.h"

#include "Session.h"

#include "JoystickDriver.h"
#include "Joystick.h"

#include <QVBoxLayout>
#include <QLabel>

#include <iostream>

SessionWidget::SessionWidget(QWidget *parent, Session* session) :
    QWidget(parent),
    m_session(session)
{
    if (m_session)
    {
        m_session->addObserver(this);
    }

    m_layout = new QVBoxLayout(this);
    setLayout(m_layout);
}

SessionWidget::~SessionWidget()
{
    if (m_session)
    {
        m_session->removeObserver(this);
        m_session = 0;
    }
}

void SessionWidget::setSession(Session *session)
{
    if (m_session == session)
    {
        return;
    }

    if (m_session)
    {
        m_session->removeObserver(this);
    }

    m_session = session;

    if (m_session)
    {
        m_session->addObserver(this);
    }
}

void SessionWidget::onJoystickDriverChanged()
{
    JoystickDriver *joystickDriver = m_session->getJoystickDriver();

    if (!joystickDriver)
    {
        return;
    }

    removeJoystickWidgets();

    JoystickDriver::JoystickCollection jColl = joystickDriver->getJoysticks();
    for (Joystick const & joystick : jColl)
    {
        unsigned int numberOfButtons = joystick.getNumberOfButtons();
        for (unsigned int i = 0; i < numberOfButtons; ++i)
        {
            QLabel *buttonWidget = new QLabel(QString("Button ") + QString::number(i+1), this);
            m_layout->addWidget(buttonWidget);
            m_addedWidgets.push_back(buttonWidget);
        }

        unsigned int numberOfAxis = joystick.getNumberOfAxis();
        for (unsigned int i = 0; i < numberOfAxis; ++i)
        {
            QLabel *axisWidget = new QLabel(QString("Axis ") + QString::number(i+1), this);
            m_layout->addWidget(axisWidget);
            m_addedWidgets.push_back(axisWidget);
        }
    }
}

void SessionWidget::onMIDIDriverChanged()
{
}

void SessionWidget::onSessionChanged()
{
    if (!m_session)
    {
        return;
    }
}

void SessionWidget::removeJoystickWidgets()
{
    for (QWidget * widget : m_addedWidgets)
    {
        m_layout->removeWidget(widget);
        delete widget;
    }

    m_addedWidgets.clear();
}
