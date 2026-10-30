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

#include "MIDIEvent.h"

MIDIEvent::MIDIEvent() :
    m_status(NoteOff),
    m_data1(0),
    m_data2(0),
    m_sysExSize(0)
{
}

MIDIEvent MIDIEvent::makeMmc(uint8_t deviceId, uint8_t command)
{
    MIDIEvent e;
    e.m_status = 0xF0;
    // Universal real-time (7F), device, MMC command (06), command
    uint8_t const bytes[] = { 0xF0, 0x7F, static_cast<uint8_t>(deviceId & 0x7F), 0x06,
                              static_cast<uint8_t>(command & 0x7F), 0xF7 };
    for (size_t i = 0; i < sizeof(bytes); ++i)
    {
        e.m_sysEx[i] = bytes[i];
    }
    e.m_sysExSize = sizeof(bytes);
    return e;
}

MIDIEvent::~MIDIEvent()
{
}
