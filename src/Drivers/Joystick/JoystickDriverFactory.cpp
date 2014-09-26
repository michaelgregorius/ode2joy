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
#include "JoystickDriverFactory.h"

#include "JoystickDriverNull.h"
#include "JoystickDriverLinux.h"


JoystickDriverFactory* JoystickDriverFactory::s_instance = 0;

static std::string s_nullKey("null");
static std::string s_linuxKey("linux");

JoystickDriverFactory::JoystickDriverFactory()
{
    m_driverDescriptions.push_back(DriverDescription(s_nullKey, "Null driver"));
    m_driverDescriptions.push_back(DriverDescription(s_linuxKey, "Linux kernel driver"));
}

JoystickDriverFactory::~JoystickDriverFactory()
{
}

JoystickDriverFactory const * JoystickDriverFactory::getInstance()
{
    static Sentinel sentinel; // Will be deleted at static time and then delete the instance.

    if (!s_instance)
    {
        s_instance = new JoystickDriverFactory();
    }

    return s_instance;
}

JoystickDriverFactory::DriverDescriptionCollection const & JoystickDriverFactory::getDriverDescriptions() const
{
    return m_driverDescriptions;
}

JoystickDriver* JoystickDriverFactory::createDriver(std::string const & key) const
{
    if (key == s_nullKey)
    {
        return new JoystickDriverNull();
    }

    if (key == s_linuxKey)
    {
        return new JoystickDriverLinux();
    }

    return 0;
}
