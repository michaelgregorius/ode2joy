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


#ifndef MIDIDRIVERFACTORY_H
#define MIDIDRIVERFACTORY_H

#include "Drivers/DriverDescription.h"

#include <string>
#include <vector>

class MIDIDriver;

/**
 * @brief Factory for MIDI drivers.
 */
class MIDIDriverFactory
{
public:

    typedef std::vector<DriverDescription> DriverDescriptionCollection;

public:
    MIDIDriverFactory();
    virtual ~MIDIDriverFactory();

    static MIDIDriverFactory const * getInstance();

    DriverDescriptionCollection const & getDriverDescriptions() const;

    MIDIDriver* createDriver(std::string const & key) const;

    // Private methods
private:
    class Sentinel
    {
    public:
        ~Sentinel()
        {
            if (MIDIDriverFactory::s_instance != 0)
            {
                delete MIDIDriverFactory::s_instance;
            }
        }
    };
    friend Sentinel;

private:
    static MIDIDriverFactory* s_instance;

    DriverDescriptionCollection m_driverDescriptions;
};

#endif
