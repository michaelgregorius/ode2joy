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


#include "JoystickEvent.h"

#include <iostream>

JoystickEvent::JoystickEvent(Type type, unsigned int number, float value, size_t id) :
    m_type(type),
    m_number(number),
    m_value(value),
    m_id(id)
{
}

JoystickEvent::~JoystickEvent()
{
}

void JoystickEvent::print() const
{
    std::cout << "Type: ";
    if (getType() == Axis)
    {
        std::cout << "Axis";
    }

    if (getType() == ButtonDown)
    {
        std::cout << "Button down";
    }

    if (getType() == ButtonUp)
    {
        std::cout << "Button up";
    }

    std::cout << "\n";

    std::cout << "Number: " << getNumber() << "\n";
    std::cout << "Value: " << getValue() << "\n";
    std::cout << std::endl;
}
