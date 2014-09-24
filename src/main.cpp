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

#include "GUI/MainWindow.h"

#include "JoystickDriver.h"
#include "Joystick.h"
#include "Drivers/Joystick/JoystickDriverLinux.h"
#include "MIDIInterfaceALSA.h"
#include "JoystickToMIDIMapper.h"

#include <QApplication>

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

    QApplication app(argc, argv);

    MainWindow mainWindow(0, joystickDriver, &mapper);
    mainWindow.resize(800, 600);
    mainWindow.show();

    // For now save the return code so that we can delete the driver
    // and the interface
    int result = app.exec();

    delete joystickDriver;
    delete midiInterface;

    return result;
}
