#ifndef ODE2JOY_CORE_MAPPINGENGINE_H_
#define ODE2JOY_CORE_MAPPINGENGINE_H_

#include "ControlEvent.h"
#include "Mapping.h"
#include "MIDI/MIDIEvent.h"

#include <map>
#include <vector>

// Not thread-safe; use from a single thread.
class MappingEngine
{
public:
    typedef std::vector<MIDIEvent> Events;

    // Replaces the device's profile; held notes of the old profile are released into out.
    void setProfile(DeviceId device, Profile const &profile, Events &out);

    // Forgets the device and releases its held notes into out.
    void removeDevice(DeviceId device, Events &out);

    // Releases held notes without removing the device.
    void releaseAll(DeviceId device, Events &out);

    void processEvent(ControlEvent const &event, Events &out);

    // Sends values that were held back by rate limiting or are still being smoothed.
    void flush(DeviceId device, uint64_t now, Events &out);

    bool hasPending(DeviceId device) const;

private:
    struct MappingState
    {
        float target = 0.0f;
        float smoothed = 0.0f;
        bool initialized = false;
        bool needsFlush = false;
        int lastSent = -1;
        uint64_t lastSentTime = 0;
        uint64_t lastTime = 0;
        bool pressed = false;
        bool noteHeld = false;
        uint8_t heldChannel = 0;
        uint8_t heldNote = 0;
    };

    struct DeviceState
    {
        Profile profile;
        std::vector<MappingState> states;
        std::map<int, float> rawAxes;
    };

    static Stick const *radialStick(Profile const &profile, int axis);
    static float effectiveAxis(DeviceState const &device, int axis);

    void processAxis(Mapping const &mapping, MappingState &state, float raw, uint64_t time, Events &out);
    void advance(Mapping const &mapping, MappingState &state, uint64_t time, Events &out);
    void processButton(Mapping const &mapping, MappingState &state, bool pressed, Events &out);
    void release(MappingState &state, Events &out);

    std::map<DeviceId, DeviceState> m_devices;
};

#endif
