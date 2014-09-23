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


#ifndef JOYSTICKDRIVER_H
#define JOYSTICKDRIVER_H


#include <string>

class Joystick;

class JoystickDriver
{
public:
    JoystickDriver();
    virtual ~JoystickDriver();

    virtual Joystick getJoystick() const = 0;

    /**
     * Initializes the driver.
     *
     * Must be called before any of the other methods is called.
     */
    virtual void init() = 0;

    virtual void start() = 0;

    virtual void stop() = 0;
};

#endif
