# ode2joy
## Introduction
*ode2joy* is intended to become an application that translates joystick input to MIDI events.

You might for example map button presses on your joystick to the following MIDI events:

* *Note on* and *note off*, e.g. to trigger drum samples.
* Perform a *program change*

The input of the axis' might be mapped to the these events:

* Pitch bends
* Change of controller values

## Current state
The current implementation only maps joystick button events to hardcoded *note on* and *note off* MIDI events.

## Dependencies
Currently *ode2joy* is dependent on the following libraries:

* [Qt 4.8](http://qt-project.org/) (required)
* ALSA (optional)

## TODOs

* Implement data structures for mappings
* Implement GUI to define the mappings
* Highlight joystick events in GUI
* Implement drivers for Windows

