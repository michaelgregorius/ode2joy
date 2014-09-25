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


#include "Drivers/MIDI/MIDIDriverAlsa.h"

#include <exception>

void printEvent(snd_seq_event_t *ev)
{
    switch (ev->type)
    {
    case SND_SEQ_EVENT_CONTROLLER:
        fprintf(stderr, "Control event on Channel %2d: %5d       \n",
                ev->data.control.channel, ev->data.control.value);
        break;
    case SND_SEQ_EVENT_PITCHBEND:
        fprintf(stderr, "Pitchbender event on Channel %2d: %5d   \n",
                ev->data.control.channel, ev->data.control.value);
        break;
    case SND_SEQ_EVENT_NOTEON:
        fprintf(stderr, "Note On event on Channel %2d: %5d       \n",
                ev->data.control.channel, ev->data.note.note);
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        fprintf(stderr, "Note Off event on Channel %2d: %5d      \n",
                ev->data.control.channel, ev->data.note.note);
        break;
    }
}

MIDIDriverALSA::MIDIDriverALSA() : MIDIDriver()
{
    // Create the sequencer
    if (snd_seq_open(&m_seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
        throw std::exception();
    }
    snd_seq_set_client_name(m_seq_handle, "ode2joy");

    // Create a port that is readable for other port, i.e. we send data
    if ((m_port_id = snd_seq_create_simple_port(m_seq_handle, "ode2joy Output",
                                                SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                                                SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
        throw std::exception();
    }
}

MIDIDriverALSA::~MIDIDriverALSA()
{
    snd_seq_close(m_seq_handle);
}

void MIDIDriverALSA::sendNoteOn()
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_noteon(&ev, 0, 48, 100);
    //ev.type = SND_SEQ_EVENT_NOTEON;
    //ev.data.control.channel = 0;
    //ev.data.note.note = 48;
    printEvent(&ev);

    // Associate the event with a source port
    snd_seq_ev_set_source(&ev, m_port_id);
    // All subscriber of or port should get the event
    snd_seq_ev_set_subs(&ev);
    // Event will be sent directly
    snd_seq_ev_set_direct(&ev);

    snd_seq_event_output_direct(m_seq_handle, &ev);
}

void MIDIDriverALSA::sendNoteOff()
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_noteoff(&ev, 0, 48, 100);
    //ev.type = SND_SEQ_EVENT_NOTEON;
    //ev.data.control.channel = 0;
    //ev.data.note.note = 48;
    printEvent(&ev);

    // Associate the event with a source port
    snd_seq_ev_set_source(&ev, m_port_id);
    // All subscriber of or port should get the event
    snd_seq_ev_set_subs(&ev);
    // Event will be sent directly
    snd_seq_ev_set_direct(&ev);

    snd_seq_event_output_direct(m_seq_handle, &ev);
}
