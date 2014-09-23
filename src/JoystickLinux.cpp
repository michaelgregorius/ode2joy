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


#include "JoystickLinux.h"

#include <iostream>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <linux/joystick.h>

#define NAME_LENGTH 256

void printEventInfo(js_event const & event) {
  // Print type
  std::cout << "Type: ";
  if (event.type & JS_EVENT_BUTTON) {
    std::cout << "Button";
  }
  if (event.type & JS_EVENT_AXIS) {
    std::cout << "Axis";
  }
  if (event.type & JS_EVENT_INIT) {
    std::cout << " (init)";
  }
  std::cout << std::endl;
  
  // Print time
  std::cout << "Time: " << event.time << std::endl;
  std::cout << "Number: " << (unsigned short) event.number << std::endl;
  std::cout << "Value: " << event.value << std::endl;
  std::cout << std::endl;
}

JoystickLinux::JoystickLinux(std::string deviceFile) : Joystick(), m_buttonState(Up)
{
  if ((m_joy_fd = open("/dev/input/js0", O_RDONLY)) < 0)
  {
    throw std::exception();
  }
  
  // Set to non-blocking reads
  fcntl(m_joy_fd, F_SETFL, O_NONBLOCK);
  
  char name[NAME_LENGTH] = "Unknown";
  ioctl(m_joy_fd, JSIOCGNAME(NAME_LENGTH), name);
  m_name = name;
  
  unsigned char axis = 0;
  unsigned char buttons = 0;
  ioctl(m_joy_fd, JSIOCGAXES, &axis);
  ioctl(m_joy_fd, JSIOCGBUTTONS, &buttons);
  
  m_numberOfAxis = (unsigned int) axis;
  m_numberOfButtons = (unsigned int) buttons;
}

JoystickLinux::~JoystickLinux()
{
  close(m_joy_fd);
}
  
std::string JoystickLinux::getName() const
{
  return m_name;
}

unsigned int JoystickLinux::getNumberOfAxis() const
{
  return m_numberOfAxis;
}

unsigned int JoystickLinux::getNumberOfButtons() const
{
  return m_numberOfButtons;
}

void JoystickLinux::processEvents()
{
  struct js_event event;
  
  while (read (m_joy_fd, &event, sizeof(struct js_event)) > 0)
  {
    if (event.type & JS_EVENT_INIT)
    {
      //printEventInfo(event);
    }
    
    if (event.type == JS_EVENT_BUTTON)
    {
      //printEventInfo(event);
      bool buttonDown = (bool) event.value;
      m_buttonState = buttonDown ? Down : Up;
    }
    
    if (event.type == JS_EVENT_AXIS)
    {
      //printEventInfo(event);
    }
  }

  // When reading events in non-blocking mode a return does not necessarily
  // mean that there was an error because -1 is returned when there are no
  // more events to read. We have to check for real errors using errno.
  if (errno != EAGAIN)
  {
    throw std::exception();
  }
}

void JoystickLinux::printEvents() const
{
  struct js_event event;
  
  while (read (m_joy_fd, &event, sizeof(struct js_event)) > 0)
  {
    if (event.type & JS_EVENT_INIT)
    {
      printEventInfo(event);
    }
    
    if (event.type == JS_EVENT_BUTTON)
    {
      printEventInfo(event);
    }
    
    if (event.type == JS_EVENT_AXIS)
    {
      printEventInfo(event);
    }
  }

  // When reading events in non-blocking mode a return does not necessarily
  // mean that there was an error because -1 is returned when there are no
  // more events to read. We have to check for real errors using errno.
  if (errno != EAGAIN)
  {
    throw std::exception();
  }
}

JoystickLinux::ButtonState JoystickLinux::getButtonState() const
{
  return m_buttonState;
}