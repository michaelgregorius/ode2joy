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

#include "JoystickDriver.h"
#include "Joystick.h"
#include "Drivers/Joystick/JoystickDriverLinux.h"
#include "MIDIInterfaceALSA.h"
#include "JoystickToMIDIMapper.h"

#include <iostream>
#include <chrono>
#include <thread>

int main (int argc, char **argv)
{
    // Create a joystick
    JoystickDriver *joystickDriver = new JoystickDriverLinux();

    joystickDriver->init();

    // Now we can fetch the joystick data
    Joystick joystick = joystickDriver->getJoystick();
    std::cout << "Joystick name: " << joystick.getName() << std::endl;
    std::cout << "Number of axis: " << joystick.getNumberOfAxis() << std::endl;
    std::cout << "Number of buttons: " << joystick.getNumberOfButtons() << std::endl;

    // Create MIDI interface
    MIDIInterface *midiInterface = new MIDIInterfaceALSA();

    JoystickToMIDIMapper mapper;
    mapper.setMIDIInterface(midiInterface);

    joystickDriver->start(&mapper);

    while (true)
    {
        std::cout << "Sleeping..." << std::endl;
        std::chrono::milliseconds duration(10000);
        std::this_thread::sleep_for(duration);
        std::cout << "Sleeping ended" << std::endl;
    }

    joystickDriver->stop();

    delete joystickDriver;
    delete midiInterface;

    return 0;
}
