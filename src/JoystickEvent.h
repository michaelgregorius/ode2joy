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


#ifndef JOYSTICKEVENT_H
#define JOYSTICKEVENT_H


#include <cstdlib>

/**
 * @brief Describes a joystick event.
 *
 * Describes the push and release of buttons and the movement of axis.
 * Axis values are supposed to be mapped to (-1, 1) by the joystick
 * driver.
 */
class JoystickEvent
{
public:
    enum Type
    {
        Axis,
        ButtonUp,
        ButtonDown
    };

    // Public methods
public:
    JoystickEvent(Type type, unsigned int number, float value, size_t id);
    ~JoystickEvent();

    /**
     * @brief getType Return the type of the event.
     * @return Type of the event.
     */
    Type getType() const { return m_type; }

    /**
     * @brief getNumber Return the number that identifies the button or axis.
     * @return The number that identifies the button or axis.
     */
    unsigned int getNumber() const { return m_number; }

    /**
     * @brief getValue Return the value of an axis, etc.
     * The value of an event lies in the interval (-1, 1).
     *
     * @return The value of an axis, etc.
     */
    float getValue() const { return m_value; }

    /**
     * @brief getId Returns the id of the joystick that generated the event.
     * @return The id of the joystick that generated the event.
     */
    size_t getId() const { return m_id; }

    void print() const;

private:
    Type m_type;
    unsigned int m_number;
    float m_value;
    /** Id of the joystick that generated the event. */
    size_t m_id;
};

#endif
