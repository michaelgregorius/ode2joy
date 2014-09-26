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


#ifndef DRIVERDESCRIPTION_H
#define DRIVERDESCRIPTION_H

#include <string>


/**
 * @brief Description for drivers.
 */
class DriverDescription
{
public:
    DriverDescription(std::string const & key, std::string const & description) :
        m_key(key),
        m_description(description)
    {
    }

    ~DriverDescription() {}

    std::string const & getKey() const { return m_key; }
    std::string const & getDescription() const { return m_description; }

private:
    std::string m_key;
    std::string m_description;
};

#endif
