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


#ifndef JOYSTICKDRIVERLINUX_H
#define JOYSTICKDRIVERLINUX_H

#include "JoystickDriver.h"


class JoystickDriverLinux : public JoystickDriver
{
public:
    JoystickDriverLinux();
    virtual ~JoystickDriverLinux();

    virtual Joystick getJoystick() const;

    /**
     * Initializes the driver.
     *
     * Must be called before any of the other methods is called.
     */
    virtual void init();

    virtual void start();

    virtual void stop();

    // Private members
private:
    int m_joy_fd; // File descriptor of the joystick

    std::string m_name;
    unsigned int m_numberOfButtons;
    unsigned int m_numberOfAxis;
};

#endif
