#ifndef ODE2JOY_CORE_CONTROLEVENT_H_
#define ODE2JOY_CORE_CONTROLEVENT_H_

#include <cstdint>
#include <string>

using DeviceId = uint32_t;

enum class ControlType
{
    Axis,
    Button,
    Hat
};

// Bits used in the value of Hat events.
enum HatDirection : int
{
    HatUp = 1,
    HatRight = 2,
    HatDown = 4,
    HatLeft = 8
};

struct DeviceControl
{
    ControlType type = ControlType::Axis;
    int index = 0;
};

struct ControlEvent
{
    DeviceId device = 0;
    ControlType type = ControlType::Axis;
    int index = 0;
    // Axis: -1..1. Button: 0 or 1. Hat: bitmask of HatDirection.
    float value = 0.0f;
    // Nanoseconds on the clock of the InputBackend that produced the event.
    uint64_t timestamp = 0;
};

struct DeviceInfo
{
    DeviceId id = 0;
    std::string name;
    std::string guid;   // Identifies the device type.
    std::string serial; // May be empty.
    int numAxes = 0;
    int numButtons = 0;
    int numHats = 0;
    // Axis indices of the sticks, -1 if unknown.
    int leftStickX = -1;
    int leftStickY = -1;
    int rightStickX = -1;
    int rightStickY = -1;
    int instance = 1;   // 1-based number among connected devices with the same GUID.
};

#endif
