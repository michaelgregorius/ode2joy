/*
 * Copyright (C) 2011 Michael Gregorius
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


#ifndef JOYSTICKTOMIDIMAPPER_H
#define JOYSTICKTOMIDIMAPPER_H


class MIDIDriver;


/**
 * @brief Performs the mapping from joystick events to MIDI events.
 */
class JoystickToMIDIMapper
{
public:
    JoystickToMIDIMapper();
    ~JoystickToMIDIMapper();

    void setMIDIDriver(MIDIDriver *midiDriver);

    void processJoystickEventButtonDown();
    void processJoystickEventButtonUp();

private:
    MIDIDriver *m_midiDriver;
};

#endif
