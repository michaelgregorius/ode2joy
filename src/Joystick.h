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


#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <string>

class Joystick
{
public:
  // Public enums
  enum ButtonState { Up, Down };
  
  // Public methods
public:
  Joystick();
  virtual ~Joystick();
  
  virtual std::string getName() const;
  
  virtual unsigned int getNumberOfAxis() const = 0;
  
  virtual unsigned int getNumberOfButtons() const = 0;
  
  // Temporary methods for testing and quick results
  virtual void processEvents() = 0;
  
  virtual void printEvents() const;
  
  virtual ButtonState getButtonState() const = 0;
};

#endif