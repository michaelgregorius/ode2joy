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


#ifndef MIDIDRIVERALSA_H
#define MIDIDRIVERALSA_H

#include "MIDIDriver.h"

#include <alsa/asoundlib.h>

class MIDIDriverALSA : public MIDIDriver
{
public:
    MIDIDriverALSA();
    virtual ~MIDIDriverALSA();

    virtual void processEvent(MIDIEvent const & midiEvent);

private:
    snd_seq_t *m_seq_handle;
    int m_port_id;
};

#endif
