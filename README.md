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
Joysticks are detected with SDL3 (including hot-plugging) and every joystick appears as its own virtual MIDI output port (`ode2joy: <joystick name>`) via RtMidi, which can be selected in a DAW.

The processing pipeline is:

```
SDL3 input -> normalized ControlEvents -> mapping engine -> MIDI events -> RtMidi output
```

The mapping engine supports axes, buttons, hats and pedals/triggers (unipolar axes) with inversion, dead zone, linear/exponential/logarithmic curves, smoothing, rate limiting and suppression of unchanged MIDI values. Buttons mapped to notes are tracked, so held notes get a *note off* when the joystick is unplugged or disabled. Targets are control change, note, pitch bend, program change and channel aftertouch.

Without a profile a joystick uses a default mapping: buttons to notes starting at 36, axes to CC 16 and up, hat X/Y to CC 80 and up.

### Profiles
Profiles are JSON files. Use *File > Save profile of selected device...* to export the active profile as a starting point, edit it by hand and load it again. A profile is assigned either to the joystick type (SDL GUID) or, if the joystick reports a serial number, to that single device. Profiles are stored in the application config directory (`~/.config/ode2joy/profiles` on Linux). A graphical mapping editor is not implemented yet.

## Dependencies
The application requires the following development tools and libraries:

* CMake 3.16 or newer
* A C++17 compiler
* Qt 6 Widgets
* SDL3
* RtMidi
* pkg-config

RtMidi virtual ports are available on Linux (ALSA/JACK) and macOS (CoreMIDI), but not on Windows.

## Getting involved in development
Here are some tips if you want to get involved in the development of *ode2joy*.

### Developing on Linux
On Debian or Ubuntu, install the required packages with:

```sh
sudo apt install git build-essential cmake pkg-config qt6-base-dev libsdl3-dev librtmidi-dev
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

## TODOs

* Implement GUI to edit the mappings, including MIDI learn
* Show the live values of the controls in the GUI
* Windows support for MIDI output (no virtual ports in RtMidi)

