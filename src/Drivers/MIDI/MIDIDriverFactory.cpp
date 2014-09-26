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


// Corresponding include file
#include "MIDIDriverFactory.h"

#include "MIDIDriverALSA.h"
#include "MIDIDriverNull.h"


MIDIDriverFactory* MIDIDriverFactory::s_instance = 0;

static std::string s_alsaKey("alsa");
static std::string s_nullKey("null");

MIDIDriverFactory::MIDIDriverFactory()
{
    m_driverDescriptions.push_back(DriverDescription(s_alsaKey, "Alsa sequencer"));
    m_driverDescriptions.push_back(DriverDescription(s_nullKey, "Null driver"));
}

MIDIDriverFactory::~MIDIDriverFactory()
{
}

MIDIDriverFactory const * MIDIDriverFactory::getInstance()
{
    static Sentinel sentinel; // Will be deleted at static time and then delete the instance.

    if (!s_instance)
    {
        s_instance = new MIDIDriverFactory();
    }

    return s_instance;
}

MIDIDriverFactory::DriverDescriptionCollection const & MIDIDriverFactory::getDriverDescriptions() const
{
    return m_driverDescriptions;
}

MIDIDriver* MIDIDriverFactory::createDriver(std::string const & key) const
{
    if (key == s_alsaKey)
    {
        return new MIDIDriverALSA();
    }

    if (key == s_nullKey)
    {
        return new MIDIDriverNull();
    }

    return 0;
}
