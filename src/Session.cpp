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


#include "Session.h"

#include "JoystickDriver.h"
#include "MIDIInterface.h"

Session::Session() :
    m_joystickDriver(0),
    m_midiInterface(0)
{
    // TODO Load previous drivers/session?
}

Session::~Session()
{
    if (m_joystickDriver)
    {
        m_joystickDriver->stop();

        delete m_joystickDriver;
        m_joystickDriver = 0;
    }

    if (m_midiInterface)
    {
        delete m_midiInterface;
        m_midiInterface = 0;
    }
}

void Session::setJoystickDriver(JoystickDriver *joystickDriver)
{
    if (m_joystickDriver)
    {
        m_joystickDriver->stop();
    }

    delete m_joystickDriver;

    m_joystickDriver = joystickDriver;

    if (m_joystickDriver)
    {
        m_joystickDriver->init();
        m_joystickDriver->start(&m_mapper);
    }
}

void Session::setMIDIInterface(MIDIInterface *midiInterface)
{
    if (m_midiInterface)
    {
        delete m_midiInterface;
    }

    m_midiInterface = midiInterface;

    m_mapper.setMIDIInterface(m_midiInterface);
}
