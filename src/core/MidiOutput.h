#ifndef ODE2JOY_CORE_MIDIOUTPUT_H_
#define ODE2JOY_CORE_MIDIOUTPUT_H_

#include "ControlEvent.h"
#include "MIDI/MIDIEvent.h"

#include <memory>
#include <string>

class MidiOutput
{
public:
    virtual ~MidiOutput() {}

    virtual void send(MIDIEvent const &event) = 0;
};

// Creates one virtual port per joystick. Returns null and sets error on failure.
std::unique_ptr<MidiOutput> createRtMidiOutput(DeviceInfo const &device, std::string &error);

#endif
