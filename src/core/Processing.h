#ifndef ODE2JOY_CORE_PROCESSING_H_
#define ODE2JOY_CORE_PROCESSING_H_

#include "Mapping.h"

#include <cstdint>

// Symmetric dead zone for values in -1..1; the remaining range is rescaled to -1..1.
float applyDeadZone(float x, float deadZone);

// Dead zone at the low end for values in 0..1; the remaining range is rescaled to 0..1.
float applyUnipolarDeadZone(float x, float deadZone);

// Maps 0..1 to 0..1; the curve is applied symmetrically to negative values.
float applyCurve(float x, Curve const &curve);

// One-pole low-pass; a non-positive time constant returns the target.
float smoothStep(float current, float target, float dtSeconds, float timeConstantMs);

float clampf(float x, float lo, float hi);

// Input in 0..1.
uint8_t quantize7(float x01);
uint16_t quantize14(float x01);

#endif
