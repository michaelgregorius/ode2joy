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

#include "MIDI/MIDIEvent.h"

#include <vector>
#include <iostream>

int const error = 1;

int test_status()
{
    MIDIEvent midiEvent;

    std::vector<MIDIEvent::Status> status = { MIDIEvent::NoteOn,
                                              MIDIEvent::NoteOff,
                                              MIDIEvent::PolyphonicAftertouch,
                                              MIDIEvent::ControlChange,
                                              MIDIEvent::ProgramChange,
                                              MIDIEvent::ChannelAftertouch,
                                              MIDIEvent::PitchBending };

    for (auto s : status)
    {
        midiEvent.setStatus(s);
        if (midiEvent.getStatus() != s)
        {
            return error;
        }
    }

    return 0;
}

int test_channel()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 16; ++i)
    {
        midiEvent.setChannel(i);
        if (midiEvent.getChannel() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_note()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setNote(i);
        if (midiEvent.getNote() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_velocity()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setVelocity(i);
        if (midiEvent.getVelocity() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_polyphonic_aftertouch()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setPolyphonicAftertouch(i);
        if (midiEvent.getPolyphonicAftertouch() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_controller()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setController(i);
        if (midiEvent.getController() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_controller_value()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setControllerValue(i);
        if (midiEvent.getControllerValue() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_program_change()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setProgramChangeInstrument(i);
        if (midiEvent.getProgramChangeInstrument() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_channel_aftertouch()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setChannelAftertouch(i);
        if (midiEvent.getChannelAftertouch() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_pitch_bend()
{
    MIDIEvent midiEvent;

    for (uint16_t i = 0; i < 16384; ++i)
    {
        midiEvent.setPitchBend(i);
        if (midiEvent.getPitchBend() != i)
        {
            return error;
        }
    }

    return 0;
}

int main (int argc, char **argv)
{
    if (argc == 2)
    {
        int errorCode = 0;
        std::string testName(argv[1]);
        if (testName == "test_status")
        {
            errorCode = test_status();
        }
        else if (testName == "test_channel")
        {
            errorCode = test_channel();
        }
        else if (testName == "test_note")
        {
            errorCode = test_note();
        }
        else if (testName == "test_velocity")
        {
            errorCode = test_velocity();
        }
        else if (testName == "test_polyphonic_aftertouch")
        {
            errorCode = test_polyphonic_aftertouch();
        }
        else if (testName == "test_controller")
        {
            errorCode = test_controller();
        }
        else if (testName == "test_controller_value")
        {
            errorCode = test_controller_value();
        }
        else if (testName == "test_program_change")
        {
            errorCode = test_program_change();
        }
        else if (testName == "test_channel_aftertouch")
        {
            errorCode = test_channel_aftertouch();
        }
        else if (testName == "test_pitch_bend")
        {
            errorCode = test_pitch_bend();
        }

        return errorCode;
    }

    return 0;
}
