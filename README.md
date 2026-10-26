# ode2joy

## Introduction
*ode2joy* is intended to become an application that translates joystick input to MIDI events.

You might for example map button presses on your joystick to the following MIDI events:

* *Note on* and *note off* events, e.g. to trigger drum samples.
* *Program change* events to switch instruments.

The input of the axis' might be mapped to:

* Pitch bends
* Controllers

## Current state
The current implementation only maps joystick button events to hardcoded *note on* and *note off* MIDI events.

An actual mapping and a configuration of the mapping via the graphical user interface is not implemented yet.

## Dependencies
The application requires the following development tools and libraries:

* CMake 3.16 or newer
* A C++17 compiler
* Qt 6 Widgets
* ALSA development files on Linux (optional; without them, only the Null MIDI driver is available)

## Getting involved in development
Here are some tips if you want to get involved in the development of *ode2joy*.

### Developing on Linux
On Debian or Ubuntu, install the required packages with:

```sh
sudo apt install git build-essential cmake qt6-base-dev
```

To build the ALSA MIDI backend as well, install its development package:

```sh
sudo apt install libasound2-dev
```

#### Cloning of the repository
Use the following command to clone the git repository:
> git clone https://github.com/michaelgregorius/ode2joy.git

#### Building the application from source
From the repository root, configure, build, and run the tests with:

```sh
cmake -S src -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Start the application with `./build/ode2joy`.

### Known caveats
When developing in VirtualBox the mouse might be reported as device */dev/input/js0*. Currently this device is hard coded as the joystick in the kernel joystick driver found at *Drivers/Joystick/JoystickDriverLinux.cpp*. If you want to develop in VirtualBox you may need to change the device in this file to */dev/input/js1* or something similar.

## TODOs

* Implement data structures for mappings
* Implement GUI to define the mappings
* Highlight joystick events in GUI
* Implement joystick and MIDI drivers for Windows

