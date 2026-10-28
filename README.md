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

Without a preset a joystick uses a default mapping: buttons to notes starting at 36, axes to CC 16 and up, hat X/Y to CC 80 and up.

### Mapping presets
Open *Edit > Mapping presets...* (Ctrl+M) to edit the mappings. The editor lists every joystick type seen so far; each type can have several presets, one of which is active. For the selected preset every axis, button and hat direction of the joystick is listed and can be mapped to a MIDI message (control change, note, pitch bend, program change or channel aftertouch) with channel and number. Axes additionally offer invert, centered or single-direction range, linear/exponential/logarithmic curves with a live preview, dead zone, smoothing and a message rate cap. Controls can be given custom labels.

On the *Sticks* tab two axes can be grouped as an analog stick, which allows a radial dead zone and copying the response settings from the X to the Y axis. Each axis still maps to its own controller, so X and Y can send different CCs.

Changes are applied to the running joysticks immediately and written to disk on OK; Cancel reverts them. Presets can be imported and exported as JSON files. The preset used by a joystick is chosen in the device list: joysticks that report a serial number get their own choice, all others share the active preset of their type. Data is stored in the application config directory (`~/.config/ode2joy/types` on Linux).


## Dependencies
The application requires the following development tools and libraries:

* CMake 3.16 or newer
* A C++17 compiler
* Qt 6 Widgets
* SDL3 (built statically from the `src/ThirdParty/SDL` git submodule)
* RtMidi
* pkg-config

RtMidi virtual ports are available on Linux (ALSA/JACK) and macOS (CoreMIDI), but not on Windows.

## Getting involved in development
Here are some tips if you want to get involved in the development of *ode2joy*.

### Developing on Linux
On Debian or Ubuntu, install the required packages with:

```sh
sudo apt install git build-essential cmake pkg-config qt6-base-dev librtmidi-dev libudev-dev
```

#### Cloning of the repository
Use the following command to clone the git repository:
> git clone --recurse-submodules https://github.com/michaelgregorius/ode2joy.git

For an existing clone run `git submodule update --init`.

#### Building the application from source
From the repository root, configure, build, and run the tests with:

```sh
cmake -S src -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Start the application with `./build/ode2joy`.

## TODOs

* MIDI learn for the mapping editor
* Show the live values of the controls in the GUI
* Windows support for MIDI output (no virtual ports in RtMidi)

