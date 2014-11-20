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


#ifndef SESSION_H
#define SESSION_H

#include "JoystickToMIDIMapper.h"

#include <set>

class JoystickDriver;
class MIDIDriver;
class SessionObserver;

/**
 * @brief Describes a session (joystick driver, MIDI driver and mappings).
 */
class Session
{
public:
    Session();
    ~Session();

    void addObserver(SessionObserver* observer);
    void removeObserver(SessionObserver* observer);

    JoystickDriver * getJoystickDriver() const { return m_joystickDriver; }

    /**
     * @brief setJoystickDriver Set the joystick driver to use.
     * @param joystickDriver The JoystickDriver to use.
     *
     * Takes ownership of the JoystickDriver. In case another driver is already
     * in use it will be stopped and deleted. The new driver will then be
     * started.
     */
    void setJoystickDriver(JoystickDriver *joystickDriver);

    /**
     * @brief setMIDIDriver Sets the MIDI driver to use.
     * @param midiDriver The MIDIDriver to use.
     *
     * Takes ownership of the driver. When called with another driver already
     * in use the running driver will be stopped and deleted.
     */
    void setMIDIDriver(MIDIDriver *midiDriver);

private:
    JoystickToMIDIMapper m_mapper;

    JoystickDriver *m_joystickDriver;
    MIDIDriver *m_midiDriver;

    typedef std::set<SessionObserver*> ObserverSet;
    ObserverSet m_sessionObservers;
};

#endif
