#include "MappingEngine.h"

#include "Processing.h"

#include <cmath>

namespace
{

MIDIEvent makeEvent(MidiTarget const &target, int value)
{
    MIDIEvent e;
    e.setChannel(target.channel);

    switch (target.kind)
    {
    case MidiTargetKind::ControlChange:
        e.setStatus(MIDIEvent::ControlChange);
        e.setController(target.number);
        e.setControllerValue(static_cast<uint8_t>(value));
        break;
    case MidiTargetKind::PitchBend:
        e.setStatus(MIDIEvent::PitchBending);
        e.setPitchBend(static_cast<uint16_t>(value));
        break;
    case MidiTargetKind::ProgramChange:
        e.setStatus(MIDIEvent::ProgramChange);
        e.setProgramChangeInstrument(static_cast<uint8_t>(value));
        break;
    case MidiTargetKind::ChannelAftertouch:
        e.setStatus(MIDIEvent::ChannelAftertouch);
        e.setChannelAftertouch(static_cast<uint8_t>(value));
        break;
    case MidiTargetKind::Note:
        break;
    }

    return e;
}

MIDIEvent makeNote(MIDIEvent::Status status, uint8_t channel, uint8_t note, uint8_t velocity)
{
    MIDIEvent e;
    e.setStatus(status);
    e.setChannel(channel);
    e.setNote(note);
    e.setVelocity(velocity);
    return e;
}

bool isContinuous(MidiTargetKind kind)
{
    return kind != MidiTargetKind::Note;
}

int quantizeFor(MidiTarget const &target, float x01)
{
    return target.kind == MidiTargetKind::PitchBend ? quantize14(x01) : quantize7(x01);
}

} // namespace

void MappingEngine::setProfile(DeviceId device, Profile const &profile, Events &out)
{
    releaseAll(device, out);

    DeviceState &state = m_devices[device];
    state.profile = profile;
    state.states.assign(profile.mappings.size(), MappingState());
}

void MappingEngine::removeDevice(DeviceId device, Events &out)
{
    releaseAll(device, out);
    m_devices.erase(device);
}

void MappingEngine::releaseAll(DeviceId device, Events &out)
{
    auto it = m_devices.find(device);
    if (it == m_devices.end())
    {
        return;
    }

    for (MappingState &s : it->second.states)
    {
        release(s, out);
        s.pressed = false;
    }
}

void MappingEngine::release(MappingState &state, Events &out)
{
    if (state.noteHeld)
    {
        out.push_back(makeNote(MIDIEvent::NoteOff, state.heldChannel, state.heldNote, 0));
        state.noteHeld = false;
    }
}

void MappingEngine::processEvent(ControlEvent const &event, Events &out)
{
    auto it = m_devices.find(event.device);
    if (it == m_devices.end())
    {
        return;
    }

    DeviceState &device = it->second;
    for (size_t i = 0; i < device.profile.mappings.size(); ++i)
    {
        Mapping const &m = device.profile.mappings[i];
        if (m.source.type != event.type || m.source.index != event.index)
        {
            continue;
        }

        MappingState &s = device.states[i];

        switch (event.type)
        {
        case ControlType::Axis:
            if (isContinuous(m.target.kind))
            {
                processAxis(m, s, event.value, event.timestamp, out);
            }
            break;

        case ControlType::Button:
            processButton(m, s, event.value > 0.5f, out);
            break;

        case ControlType::Hat:
        {
            int const mask = static_cast<int>(event.value);
            switch (m.hatComponent)
            {
            case HatComponent::X:
            case HatComponent::Y:
                if (isContinuous(m.target.kind))
                {
                    float v;
                    if (m.hatComponent == HatComponent::X)
                    {
                        v = ((mask & HatRight) ? 1.0f : 0.0f) - ((mask & HatLeft) ? 1.0f : 0.0f);
                    }
                    else
                    {
                        v = ((mask & HatUp) ? 1.0f : 0.0f) - ((mask & HatDown) ? 1.0f : 0.0f);
                    }
                    processAxis(m, s, v, event.timestamp, out);
                }
                break;
            case HatComponent::Up:
                processButton(m, s, (mask & HatUp) != 0, out);
                break;
            case HatComponent::Right:
                processButton(m, s, (mask & HatRight) != 0, out);
                break;
            case HatComponent::Down:
                processButton(m, s, (mask & HatDown) != 0, out);
                break;
            case HatComponent::Left:
                processButton(m, s, (mask & HatLeft) != 0, out);
                break;
            }
            break;
        }
        }
    }
}

void MappingEngine::processAxis(Mapping const &m, MappingState &s, float raw, uint64_t time, Events &out)
{
    float y;
    if (m.range == AxisRange::Unipolar)
    {
        y = clampf((raw + 1.0f) * 0.5f, 0.0f, 1.0f);
        if (m.invert)
        {
            y = 1.0f - y;
        }
        y = applyUnipolarDeadZone(y, m.deadZone);
        y = applyCurve(y, m.curve);
    }
    else
    {
        y = clampf(raw, -1.0f, 1.0f);
        if (m.invert)
        {
            y = -y;
        }
        y = applyDeadZone(y, m.deadZone);
        y = applyCurve(y, m.curve);
        y = (y + 1.0f) * 0.5f;
    }

    s.target = y;
    if (!s.initialized)
    {
        s.initialized = true;
        s.smoothed = y;
        s.lastTime = time;
    }

    advance(m, s, time, out);
}

void MappingEngine::advance(Mapping const &m, MappingState &s, uint64_t time, Events &out)
{
    float const dt = time > s.lastTime ? static_cast<float>(time - s.lastTime) * 1e-9f : 0.0f;
    s.lastTime = time;

    s.smoothed = smoothStep(s.smoothed, s.target, dt, m.smoothing);
    if (std::abs(s.target - s.smoothed) < 1e-4f)
    {
        s.smoothed = s.target;
    }

    int const q = quantizeFor(m.target, s.smoothed);
    bool pending = s.smoothed != s.target;

    if (q != s.lastSent)
    {
        uint64_t const minInterval = m.maxRateHz > 0.0f ? static_cast<uint64_t>(1e9f / m.maxRateHz) : 0;
        bool const first = s.lastSent < 0;
        if (first || minInterval == 0 || time < s.lastSentTime || time - s.lastSentTime >= minInterval)
        {
            out.push_back(makeEvent(m.target, q));
            s.lastSent = q;
            s.lastSentTime = time;
        }
        else
        {
            pending = true;
        }
    }

    s.needsFlush = pending;
}

void MappingEngine::processButton(Mapping const &m, MappingState &s, bool pressed, Events &out)
{
    if (m.invert)
    {
        pressed = !pressed;
    }

    if (pressed == s.pressed)
    {
        return;
    }
    s.pressed = pressed;

    switch (m.target.kind)
    {
    case MidiTargetKind::Note:
        if (pressed)
        {
            uint8_t const velocity = m.target.onValue == 0 ? 1 : m.target.onValue;
            out.push_back(makeNote(MIDIEvent::NoteOn, m.target.channel, m.target.number, velocity));
            s.noteHeld = true;
            s.heldChannel = m.target.channel;
            s.heldNote = m.target.number;
        }
        else
        {
            release(s, out);
        }
        break;

    case MidiTargetKind::ControlChange:
    case MidiTargetKind::ChannelAftertouch:
        out.push_back(makeEvent(m.target, pressed ? m.target.onValue : m.target.offValue));
        break;

    case MidiTargetKind::ProgramChange:
        if (pressed)
        {
            out.push_back(makeEvent(m.target, m.target.number));
        }
        break;

    case MidiTargetKind::PitchBend:
        break;
    }
}

void MappingEngine::flush(DeviceId device, uint64_t now, Events &out)
{
    auto it = m_devices.find(device);
    if (it == m_devices.end())
    {
        return;
    }

    DeviceState &d = it->second;
    for (size_t i = 0; i < d.states.size(); ++i)
    {
        if (d.states[i].needsFlush)
        {
            advance(d.profile.mappings[i], d.states[i], now, out);
        }
    }
}

bool MappingEngine::hasPending(DeviceId device) const
{
    auto it = m_devices.find(device);
    if (it == m_devices.end())
    {
        return false;
    }

    for (MappingState const &s : it->second.states)
    {
        if (s.needsFlush)
        {
            return true;
        }
    }
    return false;
}
