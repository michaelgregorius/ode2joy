#ifndef ODE2JOY_CORE_INPUTBACKEND_H_
#define ODE2JOY_CORE_INPUTBACKEND_H_

#include "ControlEvent.h"

#include <string>

class InputListener
{
public:
    virtual ~InputListener() {}

    virtual void onDeviceAdded(DeviceInfo const &info) = 0;
    virtual void onDeviceRemoved(DeviceId id) = 0;
    virtual void onControlEvent(ControlEvent const &event) = 0;
};

// All methods must be called from the same thread.
class InputBackend
{
public:
    virtual ~InputBackend() {}

    virtual bool init(std::string &error) = 0;
    virtual void shutdown() = 0;

    // Waits up to timeoutMs for events and dispatches everything that is available.
    virtual void poll(int timeoutMs, InputListener &listener) = 0;

    // Monotonic nanoseconds on the clock used for ControlEvent::timestamp.
    virtual uint64_t nowNs() const = 0;
};

#endif
