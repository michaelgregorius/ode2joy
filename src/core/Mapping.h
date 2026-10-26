#ifndef ODE2JOY_CORE_MAPPING_H_
#define ODE2JOY_CORE_MAPPING_H_

#include "ControlEvent.h"

#include <cstdint>
#include <string>
#include <vector>

enum class MidiTargetKind
{
    ControlChange,
    Note,
    PitchBend,
    ProgramChange,
    ChannelAftertouch
};

struct MidiTarget
{
    MidiTargetKind kind = MidiTargetKind::ControlChange;
    uint8_t channel = 0;  // 0-based
    uint8_t number = 0;   // Controller, note or program number
    uint8_t onValue = 127;  // Button pressed: velocity or CC value
    uint8_t offValue = 0;   // Button released: CC value
};

enum class CurveType
{
    Linear,
    Exponential,
    Logarithmic
};

struct Curve
{
    CurveType type = CurveType::Linear;
    float amount = 3.0f; // Steepness; ignored for Linear
};

enum class AxisRange
{
    Bipolar,  // -1..1, e.g. a stick
    Unipolar  // Raw -1..1 is treated as 0..1, e.g. a pedal or trigger
};

enum class HatComponent
{
    X, Y, Up, Right, Down, Left
};

struct Mapping
{
    DeviceControl source;
    HatComponent hatComponent = HatComponent::X; // Only used for Hat sources
    MidiTarget target;
    AxisRange range = AxisRange::Bipolar;
    Curve curve;
    float deadZone = 0.0f;   // 0..<1
    float smoothing = 0.0f;  // Low-pass time constant in milliseconds; 0 disables
    bool invert = false;
    float maxRateHz = 250.0f; // Upper bound for messages per second; 0 disables
};

struct Profile
{
    std::string name;
    std::string deviceGuid;
    std::string deviceSerial; // Non-empty: applies only to this physical device
    std::vector<Mapping> mappings;
};

#endif
