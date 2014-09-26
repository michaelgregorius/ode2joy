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


#ifndef JOYSTICKDRIVERFACTORY_H
#define JOYSTICKDRIVERFACTORY_H

#include "Drivers/DriverDescription.h"

#include <string>
#include <vector>

class JoystickDriver;

/**
 * @brief Factory for joystick drivers.
 */
class JoystickDriverFactory
{
public:

    typedef std::vector<DriverDescription> DriverDescriptionCollection;

public:
    JoystickDriverFactory();
    virtual ~JoystickDriverFactory();

    static JoystickDriverFactory const * getInstance();

    DriverDescriptionCollection const & getDriverDescriptions() const;

    JoystickDriver* createDriver(std::string const & key) const;

    // Private methods
private:
    class Sentinel
    {
    public:
        ~Sentinel()
        {
            if (JoystickDriverFactory::s_instance != 0)
            {
                delete JoystickDriverFactory::s_instance;
            }
        }
    };
    friend Sentinel;

private:
    static JoystickDriverFactory* s_instance;

    DriverDescriptionCollection m_driverDescriptions;
};

#endif
