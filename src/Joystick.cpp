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


#include "Joystick.h"

Joystick::Joystick(size_t id, std::string const & name, unsigned int numberOfAxis, unsigned int numberOfButtons) :
    m_id(id),
    m_name(name),
    m_numberOfAxis(numberOfAxis),
    m_numberOfButtons(numberOfButtons)
{
}

Joystick::~Joystick()
{
}

std::string Joystick::getName() const
{
    return m_name;
}

unsigned int Joystick::getNumberOfAxis() const
{
    return m_numberOfAxis;
}

unsigned int Joystick::getNumberOfButtons() const
{
    return m_numberOfButtons;
}
