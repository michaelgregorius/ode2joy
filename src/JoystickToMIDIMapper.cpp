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


#include "JoystickToMIDIMapper.h"

#include "MIDIDriver.h"
#include "JoystickEvent.h"

#include "MIDI/MIDIEvent.h"

JoystickToMIDIMapper::JoystickToMIDIMapper() :
    m_midiDriver(0)
{
}

JoystickToMIDIMapper::~JoystickToMIDIMapper()
{
}

void JoystickToMIDIMapper::setMIDIDriver(MIDIDriver *midiDriver)
{
    m_midiDriver = midiDriver;
}

void JoystickToMIDIMapper::handleJoystickEvent(JoystickEvent event)
{
    event.print();
    if (event.getType() == JoystickEvent::ButtonDown)
    {
        if (m_midiDriver)
        {
            MIDIEvent midiEvent;
            midiEvent.setStatus(MIDIEvent::NoteOn);
            midiEvent.setChannel(0);
            midiEvent.setNote(48 - event.getNumber());
            midiEvent.setVelocity(100);
            m_midiDriver->processEvent(midiEvent);
        }
    }
    else if (event.getType() == JoystickEvent::ButtonUp)
    {
        if (m_midiDriver)
        {
            MIDIEvent midiEvent;
            midiEvent.setStatus(MIDIEvent::NoteOff);
            midiEvent.setChannel(0);
            midiEvent.setNote(48 - event.getNumber());
            midiEvent.setVelocity(100);
            m_midiDriver->processEvent(midiEvent);
        }
    }
}
