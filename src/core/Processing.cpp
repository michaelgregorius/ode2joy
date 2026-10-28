#include "Processing.h"

#include <cmath>

float clampf(float x, float lo, float hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

float applyDeadZone(float x, float deadZone)
{
    deadZone = clampf(deadZone, 0.0f, 0.99f);
    if (std::abs(x) < deadZone)
    {
        return 0.0f;
    }

    float const sign = x < 0.0f ? -1.0f : 1.0f;
    return sign * (std::abs(x) - deadZone) / (1.0f - deadZone);
}

float applyUnipolarDeadZone(float x, float deadZone)
{
    deadZone = clampf(deadZone, 0.0f, 0.99f);
    if (x <= deadZone)
    {
        return 0.0f;
    }

    return (x - deadZone) / (1.0f - deadZone);
}

float applyCurve(float x, Curve const &curve)
{
    float const k = curve.amount;
    if (curve.type == CurveType::Linear || k < 0.001f)
    {
        return x;
    }

    float const sign = x < 0.0f ? -1.0f : 1.0f;
    float const a = clampf(std::abs(x), 0.0f, 1.0f);
    float const e = std::exp(k) - 1.0f;

    if (curve.type == CurveType::Exponential)
    {
        return sign * (std::exp(k * a) - 1.0f) / e;
    }

    return sign * std::log1p(e * a) / k;
}

float smoothStep(float current, float target, float dtSeconds, float timeConstantMs)
{
    if (timeConstantMs <= 0.0f)
    {
        return target;
    }

    float const alpha = 1.0f - std::exp(-dtSeconds * 1000.0f / timeConstantMs);
    return current + alpha * (target - current);
}

float shapeAxis(Mapping const &m, float raw)
{
    if (m.range == AxisRange::Unipolar)
    {
        float y = clampf((raw + 1.0f) * 0.5f, 0.0f, 1.0f);
        if (m.invert)
        {
            y = 1.0f - y;
        }
        return applyCurve(applyUnipolarDeadZone(y, m.deadZone), m.curve);
    }

    float y = clampf(raw, -1.0f, 1.0f);
    if (m.invert)
    {
        y = -y;
    }
    y = applyCurve(applyDeadZone(y, m.deadZone), m.curve);
    return (y + 1.0f) * 0.5f;
}

uint8_t quantize7(float x01)
{
    return static_cast<uint8_t>(std::lround(clampf(x01, 0.0f, 1.0f) * 127.0f));
}

uint16_t quantize14(float x01)
{
    return static_cast<uint16_t>(std::lround(clampf(x01, 0.0f, 1.0f) * 16383.0f));
}
