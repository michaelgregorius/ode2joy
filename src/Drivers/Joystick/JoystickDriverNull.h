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


#ifndef JOYSTICKDRIVERNULL_H
#define JOYSTICKDRIVERNULL_H

#include "JoystickDriver.h"


/**
 * @brief Implements a null driver that given no input.
 */
class JoystickDriverNull : public JoystickDriver
{
public:
    JoystickDriverNull();
    virtual ~JoystickDriverNull();

    virtual JoystickCollection getJoysticks() const;

    /**
     * Initializes the driver.
     *
     * Must be called before any of the other methods is called.
     */
    virtual void init();

    virtual void start(JoystickToMIDIMapper *joystickToMIDIMapper);

    virtual void stop();
};

#endif
