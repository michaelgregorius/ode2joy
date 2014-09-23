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


#include <iostream>

#include "JoystickLinux.h"
#include "MIDIInterfaceALSA.h"

int main (int argc, char **argv)
{
  // Create a joystick
  Joystick *joystick = new JoystickLinux("/dev/input/js0");

  std::cout << "Joystick name: " << joystick->getName() << std::endl;
  std::cout << "Number of axis: " << joystick->getNumberOfAxis() << std::endl;
  std::cout << "Number of buttons: " << joystick->getNumberOfButtons() << std::endl;
  
  // Create MIDI interface
  MIDIInterface *midiInterface = new MIDIInterfaceALSA();

  bool lastSendUp = true;
  
  while(true) { // Remove for single shot
    joystick->processEvents();

    Joystick::ButtonState state = joystick->getButtonState();
    switch (state)
    {
      case Joystick::Down:
        {
          if (lastSendUp)
          {
            std::cout << "Sending note on..." << std::endl;
            midiInterface->sendNoteOn();
            lastSendUp = false;
          }
        }
        break;
      case Joystick::Up:
        {
          if (!lastSendUp)
          {
            std::cout << "Sending a note off..." << std::endl;
            midiInterface->sendNoteOff();
            lastSendUp = true;
          }
        }
        break;
    }
    
    //std::cout << "Going to sleep..." << std::endl;
    //usleep(1000000);
  }
  
  delete joystick;
  delete midiInterface;
  
  return 0;
}