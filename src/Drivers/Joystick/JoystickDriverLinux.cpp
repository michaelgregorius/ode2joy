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


// Corresponding include file
#include "JoystickDriverLinux.h"

#include "Joystick.h"
#include "JoystickEvent.h"
#include "JoystickToMIDIMapper.h"

#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <linux/joystick.h>

#include <thread>
#include <chrono>
#include <atomic>


#define NAME_LENGTH 256


std::atomic<bool> started(false);

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

inline float mapToFloat(int value)
{
    return (static_cast<float>(value) / 32767);
}

void process_in_thread(int joy_fd, JoystickToMIDIMapper *joystickToMIDIMapper)
{
    struct js_event event;

    while (started)
    {
        while (read (joy_fd, &event, sizeof(struct js_event)) > 0)
        {
            if (event.type & JS_EVENT_INIT)
            {
                // TODO Support init events in JoystickEvent?
                printEventInfo(event);
            }

            if (event.type == JS_EVENT_BUTTON)
            {
                //printEventInfo(event);
                bool buttonDown = static_cast<bool>(event.value);
                JoystickEvent::Type eventType = buttonDown ? JoystickEvent::ButtonDown : JoystickEvent::ButtonUp;
                joystickToMIDIMapper->handleJoystickEvent(JoystickEvent(eventType, event.number, event.value, joy_fd));
            }

            if (event.type == JS_EVENT_AXIS)
            {
                //printEventInfo(event);
                float value = mapToFloat(event.value);
                joystickToMIDIMapper->handleJoystickEvent(JoystickEvent(JoystickEvent::Axis, event.number, value, joy_fd));
            }
        }

        // When reading events in non-blocking mode a return does not necessarily
        // mean that there was an error because -1 is returned when there are no
        // more events to read. We have to check for real errors using errno.
        if (errno != EAGAIN)
        {
            // TODO Is executed when the joystick is pulled out. Handle correctly!
            throw std::exception();
        }

        // TODO MGR: Is this the way to do it? It seems clumsy (but it works).
        std::chrono::milliseconds duration(5);
        std::this_thread::sleep_for(duration);
    }
}

std::thread processThread;

JoystickDriverLinux::JoystickDriverLinux() :
    m_joy_fd(0),
    m_initialized(false)
{
    if ((m_joy_fd = open("/dev/input/js0", O_RDONLY)) < 0)
    {
        return; // This will keep the driver uninitialized and reporting 0 joysticks
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

    m_initialized = true;
}

JoystickDriverLinux::~JoystickDriverLinux()
{
    close(m_joy_fd);
}

JoystickDriver::JoystickCollection JoystickDriverLinux::getJoysticks() const
{
    if (!m_initialized)
    {
        return JoystickCollection();
    }
    JoystickCollection joysticks = { Joystick(m_joy_fd, m_name, m_numberOfAxis, m_numberOfButtons) };
    return joysticks;
}

void JoystickDriverLinux::init()
{
    // All done in the constructor
}

void JoystickDriverLinux::start(JoystickToMIDIMapper *joystickToMIDIMapper)
{
    if (!m_initialized)
    {
        return;
    }

    started = true;

    processThread = std::thread(process_in_thread, m_joy_fd, joystickToMIDIMapper);
}

void JoystickDriverLinux::stop()
{
    if (!m_initialized)
    {
        return;
    }

    started = false;

    processThread.join();
}
