/*
 * Copyright (C) 2014 Michael Gregorius
 *
 * This file is part of ode2joy.
 *
 * ode2joy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ode2joy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ode2joy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MIDI/MIDIEvent.h"
#include "core/MappingEngine.h"
#include "core/Pipeline.h"
#include "core/Processing.h"

#include <chrono>
#include <cmath>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

int const error = 1;

int test_status()
{
    MIDIEvent midiEvent;

    std::vector<MIDIEvent::Status> status = { MIDIEvent::NoteOn,
                                              MIDIEvent::NoteOff,
                                              MIDIEvent::PolyphonicAftertouch,
                                              MIDIEvent::ControlChange,
                                              MIDIEvent::ProgramChange,
                                              MIDIEvent::ChannelAftertouch,
                                              MIDIEvent::PitchBending };

    for (auto s : status)
    {
        midiEvent.setStatus(s);
        if (midiEvent.getStatus() != s)
        {
            return error;
        }
    }

    return 0;
}

int test_channel()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 16; ++i)
    {
        midiEvent.setChannel(i);
        if (midiEvent.getChannel() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_note()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setNote(i);
        if (midiEvent.getNote() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_velocity()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setVelocity(i);
        if (midiEvent.getVelocity() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_polyphonic_aftertouch()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setPolyphonicAftertouch(i);
        if (midiEvent.getPolyphonicAftertouch() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_controller()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setController(i);
        if (midiEvent.getController() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_controller_value()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setControllerValue(i);
        if (midiEvent.getControllerValue() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_program_change()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setProgramChangeInstrument(i);
        if (midiEvent.getProgramChangeInstrument() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_channel_aftertouch()
{
    MIDIEvent midiEvent;

    for (uint8_t i = 0; i < 128; ++i)
    {
        midiEvent.setChannelAftertouch(i);
        if (midiEvent.getChannelAftertouch() != i)
        {
            return error;
        }
    }

    return 0;
}

int test_pitch_bend()
{
    MIDIEvent midiEvent;

    for (uint16_t i = 0; i < 16384; ++i)
    {
        midiEvent.setPitchBend(i);
        if (midiEvent.getPitchBend() != i)
        {
            return error;
        }
    }

    return 0;
}

// ---- Processing ----

bool near(float a, float b, float eps = 1e-4f)
{
    return std::abs(a - b) <= eps;
}

int test_dead_zone()
{
    if (applyDeadZone(0.04f, 0.1f) != 0.0f || applyDeadZone(-0.09f, 0.1f) != 0.0f)
    {
        return error;
    }
    if (!near(applyDeadZone(1.0f, 0.1f), 1.0f) || !near(applyDeadZone(-1.0f, 0.1f), -1.0f))
    {
        return error;
    }
    if (!near(applyDeadZone(0.55f, 0.1f), 0.5f) || !near(applyDeadZone(-0.55f, 0.1f), -0.5f))
    {
        return error;
    }
    if (applyUnipolarDeadZone(0.05f, 0.1f) != 0.0f || !near(applyUnipolarDeadZone(1.0f, 0.1f), 1.0f))
    {
        return error;
    }

    return 0;
}

int test_curves()
{
    Curve exponential = { CurveType::Exponential, 3.0f };
    Curve logarithmic = { CurveType::Logarithmic, 3.0f };
    Curve linear;

    for (Curve const &c : { exponential, logarithmic, linear })
    {
        if (!near(applyCurve(0.0f, c), 0.0f) || !near(applyCurve(1.0f, c), 1.0f) || !near(applyCurve(-1.0f, c), -1.0f))
        {
            return error;
        }
    }

    if (!(applyCurve(0.5f, exponential) < 0.5f) || !(applyCurve(0.5f, logarithmic) > 0.5f))
    {
        return error;
    }
    if (!near(applyCurve(-0.5f, exponential), -applyCurve(0.5f, exponential)))
    {
        return error;
    }
    // The curves are inverses of each other.
    return near(applyCurve(applyCurve(0.3f, exponential), logarithmic), 0.3f) ? 0 : error;
}

int test_quantize()
{
    if (quantize7(0.0f) != 0 || quantize7(1.0f) != 127 || quantize7(2.0f) != 127 || quantize7(-1.0f) != 0)
    {
        return error;
    }
    return (quantize14(1.0f) == 16383 && quantize14(0.0f) == 0) ? 0 : error;
}

// ---- Mapping engine ----

namespace
{

DeviceId const dev = 1;
uint64_t const ms = 1000000;

Mapping axisToCC(int axis, int cc)
{
    Mapping m;
    m.source.type = ControlType::Axis;
    m.source.index = axis;
    m.target.kind = MidiTargetKind::ControlChange;
    m.target.number = static_cast<uint8_t>(cc);
    m.maxRateHz = 0.0f;
    return m;
}

Mapping buttonToNote(int button, int note)
{
    Mapping m;
    m.source.type = ControlType::Button;
    m.source.index = button;
    m.target.kind = MidiTargetKind::Note;
    m.target.number = static_cast<uint8_t>(note);
    return m;
}

ControlEvent ev(ControlType type, int index, float value, uint64_t t)
{
    ControlEvent e;
    e.device = dev;
    e.type = type;
    e.index = index;
    e.value = value;
    e.timestamp = t;
    return e;
}

MappingEngine::Events run(MappingEngine &engine, ControlEvent const &e)
{
    MappingEngine::Events out;
    engine.processEvent(e, out);
    return out;
}

void install(MappingEngine &engine, std::vector<Mapping> const &mappings)
{
    Profile p;
    p.mappings = mappings;
    MappingEngine::Events out;
    engine.setProfile(dev, p, out);
}

} // namespace

int test_axis_to_cc()
{
    MappingEngine engine;
    install(engine, { axisToCC(2, 74) });

    MappingEngine::Events out = run(engine, ev(ControlType::Axis, 2, 1.0f, 0));
    if (out.size() != 1 || out[0].getStatus() != MIDIEvent::ControlChange || out[0].getController() != 74
        || out[0].getControllerValue() != 127)
    {
        return error;
    }

    out = run(engine, ev(ControlType::Axis, 2, -1.0f, ms));
    if (out.size() != 1 || out[0].getControllerValue() != 0)
    {
        return error;
    }

    out = run(engine, ev(ControlType::Axis, 2, 0.0f, 2 * ms));
    if (out.size() != 1 || out[0].getControllerValue() != 64)
    {
        return error;
    }

    // Other axes and other devices are ignored.
    if (!run(engine, ev(ControlType::Axis, 3, 1.0f, 3 * ms)).empty())
    {
        return error;
    }
    ControlEvent other = ev(ControlType::Axis, 2, 1.0f, 3 * ms);
    other.device = 99;
    return run(engine, other).empty() ? 0 : error;
}

int test_invert_and_dead_zone()
{
    Mapping m = axisToCC(0, 1);
    m.invert = true;
    m.deadZone = 0.1f;

    MappingEngine engine;
    install(engine, { m });

    MappingEngine::Events out = run(engine, ev(ControlType::Axis, 0, 1.0f, 0));
    if (out.size() != 1 || out[0].getControllerValue() != 0)
    {
        return error;
    }

    out = run(engine, ev(ControlType::Axis, 0, 0.05f, ms));
    return (out.size() == 1 && out[0].getControllerValue() == 64) ? 0 : error;
}

int test_unipolar_axis()
{
    Mapping m = axisToCC(0, 1);
    m.range = AxisRange::Unipolar;

    MappingEngine engine;
    install(engine, { m });

    MappingEngine::Events out = run(engine, ev(ControlType::Axis, 0, -1.0f, 0));
    if (out.size() != 1 || out[0].getControllerValue() != 0)
    {
        return error;
    }

    out = run(engine, ev(ControlType::Axis, 0, 1.0f, ms));
    return (out.size() == 1 && out[0].getControllerValue() == 127) ? 0 : error;
}

int test_change_suppression()
{
    MappingEngine engine;
    install(engine, { axisToCC(0, 1) });

    if (run(engine, ev(ControlType::Axis, 0, 0.5f, 0)).size() != 1)
    {
        return error;
    }

    // A raw change far below one MIDI step must not produce a message.
    if (!run(engine, ev(ControlType::Axis, 0, 0.5001f, ms)).empty())
    {
        return error;
    }

    return run(engine, ev(ControlType::Axis, 0, 0.8f, 2 * ms)).size() == 1 ? 0 : error;
}

int test_rate_limit_and_flush()
{
    Mapping m = axisToCC(0, 1);
    m.maxRateHz = 100.0f; // 10 ms

    MappingEngine engine;
    install(engine, { m });

    if (run(engine, ev(ControlType::Axis, 0, -1.0f, 0)).size() != 1)
    {
        return error;
    }
    if (!run(engine, ev(ControlType::Axis, 0, 1.0f, 2 * ms)).empty() || !engine.hasPending(dev))
    {
        return error;
    }

    MappingEngine::Events out;
    engine.flush(dev, 5 * ms, out);
    if (!out.empty())
    {
        return error;
    }

    engine.flush(dev, 12 * ms, out);
    if (out.size() != 1 || out[0].getControllerValue() != 127 || engine.hasPending(dev))
    {
        return error;
    }

    return 0;
}

int test_smoothing()
{
    Mapping m = axisToCC(0, 1);
    m.smoothing = 20.0f;

    MappingEngine engine;
    install(engine, { m });

    run(engine, ev(ControlType::Axis, 0, -1.0f, 0));
    MappingEngine::Events out = run(engine, ev(ControlType::Axis, 0, 1.0f, ms));
    if (!out.empty() && out[0].getControllerValue() > 20)
    {
        return error; // Must not jump to the target at once
    }

    int last = 0;
    for (uint64_t t = 2; t < 400; ++t)
    {
        out.clear();
        engine.flush(dev, t * ms, out);
        for (MIDIEvent const &e : out)
        {
            if (e.getControllerValue() < last)
            {
                return error;
            }
            last = e.getControllerValue();
        }
    }

    return (last == 127 && !engine.hasPending(dev)) ? 0 : error;
}

int test_pitch_bend_target()
{
    Mapping m = axisToCC(0, 0);
    m.target.kind = MidiTargetKind::PitchBend;

    MappingEngine engine;
    install(engine, { m });

    MappingEngine::Events out = run(engine, ev(ControlType::Axis, 0, 1.0f, 0));
    if (out.size() != 1 || out[0].getStatus() != MIDIEvent::PitchBending || out[0].getPitchBend() != 16383)
    {
        return error;
    }

    out = run(engine, ev(ControlType::Axis, 0, -1.0f, ms));
    return (out.size() == 1 && out[0].getPitchBend() == 0) ? 0 : error;
}

int test_button_note_tracking()
{
    Mapping m = buttonToNote(0, 60);
    m.target.channel = 2;
    m.target.onValue = 90;

    MappingEngine engine;
    install(engine, { m });

    MappingEngine::Events out = run(engine, ev(ControlType::Button, 0, 1.0f, 0));
    if (out.size() != 1 || out[0].getStatus() != MIDIEvent::NoteOn || out[0].getNote() != 60
        || out[0].getChannel() != 2 || out[0].getVelocity() != 90)
    {
        return error;
    }

    // Repeated press events do not retrigger.
    if (!run(engine, ev(ControlType::Button, 0, 1.0f, ms)).empty())
    {
        return error;
    }

    out = run(engine, ev(ControlType::Button, 0, 0.0f, 2 * ms));
    return (out.size() == 1 && out[0].getStatus() == MIDIEvent::NoteOff && out[0].getNote() == 60) ? 0 : error;
}

int test_release_on_remove()
{
    MappingEngine engine;
    install(engine, { buttonToNote(0, 60), buttonToNote(1, 62) });

    run(engine, ev(ControlType::Button, 0, 1.0f, 0));
    run(engine, ev(ControlType::Button, 1, 1.0f, 0));
    run(engine, ev(ControlType::Button, 1, 0.0f, ms));

    MappingEngine::Events out;
    engine.removeDevice(dev, out);
    if (out.size() != 1 || out[0].getStatus() != MIDIEvent::NoteOff || out[0].getNote() != 60)
    {
        return error;
    }

    out.clear();
    engine.removeDevice(dev, out);
    return out.empty() ? 0 : error;
}

int test_hat_mappings()
{
    Mapping x = axisToCC(0, 10);
    x.source.type = ControlType::Hat;
    x.hatComponent = HatComponent::X;

    Mapping up = buttonToNote(0, 40);
    up.source.type = ControlType::Hat;
    up.hatComponent = HatComponent::Up;

    MappingEngine engine;
    install(engine, { x, up });

    MappingEngine::Events out = run(engine, ev(ControlType::Hat, 0, static_cast<float>(HatRight | HatUp), 0));
    if (out.size() != 2 || out[0].getControllerValue() != 127 || out[1].getStatus() != MIDIEvent::NoteOn)
    {
        return error;
    }

    out = run(engine, ev(ControlType::Hat, 0, static_cast<float>(HatLeft), ms));
    if (out.size() != 2 || out[0].getControllerValue() != 0 || out[1].getStatus() != MIDIEvent::NoteOff)
    {
        return error;
    }

    return 0;
}

// ---- Pipeline ----

namespace
{

class FakeInput : public InputBackend
{
public:
    bool init(std::string &) override { return true; }
    void shutdown() override {}
    uint64_t nowNs() const override
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    void poll(int timeoutMs, InputListener &listener) override
    {
        std::vector<std::function<void(InputListener &)>> steps;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            steps.swap(m_steps);
        }
        for (auto &s : steps)
        {
            s(listener);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
    }

    void push(std::function<void(InputListener &)> step)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_steps.push_back(step);
    }

private:
    std::mutex m_mutex;
    std::vector<std::function<void(InputListener &)>> m_steps;
};

struct Sent
{
    std::mutex mutex;
    std::vector<MIDIEvent> events;

    size_t count()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return events.size();
    }

    bool waitFor(size_t n)
    {
        for (int i = 0; i < 500; ++i)
        {
            if (count() >= n)
            {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }
        return false;
    }
};

class RecordingOutput : public MidiOutput
{
public:
    explicit RecordingOutput(Sent &sent) : m_sent(sent) {}

    void send(MIDIEvent const &e) override
    {
        std::lock_guard<std::mutex> lock(m_sent.mutex);
        m_sent.events.push_back(e);
    }

private:
    Sent &m_sent;
};

} // namespace

int test_pipeline_disconnect_releases_notes()
{
    Sent sent;
    FakeInput *input = new FakeInput();
    auto library = std::make_shared<ProfileLibrary>();

    Pipeline pipeline(std::unique_ptr<InputBackend>(input),
                      [&sent](DeviceInfo const &, std::string &) { return std::unique_ptr<MidiOutput>(new RecordingOutput(sent)); },
                      library);

    std::string err;
    if (!pipeline.start(err))
    {
        return error;
    }

    DeviceInfo info;
    info.id = dev;
    info.name = "Fake";
    info.guid = "fake-guid";
    info.numButtons = 2;

    input->push([info](InputListener &l) { l.onDeviceAdded(info); });
    input->push([](InputListener &l) { l.onControlEvent(ev(ControlType::Button, 0, 1.0f, 1)); });

    // Default profile maps button 0 to note 36.
    if (!sent.waitFor(1))
    {
        return error;
    }

    input->push([](InputListener &l) { l.onDeviceRemoved(dev); });
    if (!sent.waitFor(2))
    {
        return error;
    }

    pipeline.stop();

    std::lock_guard<std::mutex> lock(sent.mutex);
    return (sent.events.size() == 2 && sent.events[0].getStatus() == MIDIEvent::NoteOn
            && sent.events[1].getStatus() == MIDIEvent::NoteOff && sent.events[1].getNote() == 36) ? 0 : error;
}

int test_profile_library_resolution()
{
    ProfileLibrary library;
    DeviceInfo info;
    info.guid = "g";
    info.name = "Pad";
    info.serial = "s1";
    info.numAxes = 2;

    // Unknown type: built-in default.
    if (library.resolve(info).name != "Default" || !library.selectedPresetName(info).empty())
    {
        return error;
    }

    if (!library.registerType(deviceTypeOf(info)) || library.registerType(deviceTypeOf(info)))
    {
        return error;
    }

    TypeRecord record;
    if (!library.getRecord("g", record) || record.type.numAxes != 2)
    {
        return error;
    }

    Profile a;
    a.name = "A";
    Profile b;
    b.name = "B";
    record.presets = { a, b };
    record.activePreset = "B";
    library.setRecord(record);

    if (library.resolve(info).name != "B")
    {
        return error;
    }

    library.setInstancePreset("g", "s1", "A");
    if (library.resolve(info).name != "A" || library.selectedPresetName(info) != "A")
    {
        return error;
    }

    // Other physical devices of the type keep the active preset.
    info.serial = "s2";
    if (library.resolve(info).name != "B")
    {
        return error;
    }

    // An unknown preset name falls back to the first preset.
    record.activePreset = "gone";
    library.setRecord(record);
    return library.resolve(info).name == "A" ? 0 : error;
}

int test_default_profile_axes()
{
    DeviceType type;
    type.guid = "g";
    type.numAxes = 4;
    type.numButtons = 2;
    type.numHats = 1;

    Profile p = makeDefaultProfile(type);
    if (p.sticks.size() != 2 || p.sticks[0].xAxis != 0 || p.sticks[0].yAxis != 1)
    {
        return error;
    }

    // Every axis gets its own controller.
    std::map<int, int> controllers;
    for (Mapping const &m : p.mappings)
    {
        if (m.source.type == ControlType::Axis)
        {
            ++controllers[m.target.number];
        }
    }
    return controllers.size() == 4 ? 0 : error;
}

int test_shape_axis()
{
    Mapping m;
    m.deadZone = 0.1f;
    m.invert = true;
    m.curve = { CurveType::Exponential, 3.0f };

    if (!near(shapeAxis(m, 0.0f), 0.5f) || !near(shapeAxis(m, 1.0f), 0.0f) || !near(shapeAxis(m, -1.0f), 1.0f))
    {
        return error;
    }

    m.range = AxisRange::Unipolar;
    m.invert = false;
    m.deadZone = 0.0f;
    m.curve = Curve();
    return (near(shapeAxis(m, -1.0f), 0.0f) && near(shapeAxis(m, 1.0f), 1.0f)) ? 0 : error;
}

int test_x_and_y_use_different_ccs()
{
    MappingEngine engine;
    install(engine, { axisToCC(0, 20), axisToCC(1, 21) });

    MappingEngine::Events out = run(engine, ev(ControlType::Axis, 0, 1.0f, 0));
    if (out.size() != 1 || out[0].getController() != 20)
    {
        return error;
    }

    out = run(engine, ev(ControlType::Axis, 1, 1.0f, ms));
    return (out.size() == 1 && out[0].getController() == 21) ? 0 : error;
}

int test_radial_dead_zone()
{
    Profile p;
    p.mappings = { axisToCC(0, 20), axisToCC(1, 21) };
    Stick s;
    s.xAxis = 0;
    s.yAxis = 1;
    s.radialDeadZone = 0.3f;
    p.sticks.push_back(s);

    MappingEngine engine;
    MappingEngine::Events out;
    engine.setProfile(dev, p, out);

    // Each axis is below 0.3, but the diagonal radius 0.35 exceeds the dead zone.
    out = run(engine, ev(ControlType::Axis, 0, 0.0f, 0));
    out = run(engine, ev(ControlType::Axis, 1, 0.0f, 0));
    out = run(engine, ev(ControlType::Axis, 0, 0.25f, ms));
    int sent = 0;
    for (MIDIEvent const &e : out)
    {
        if (e.getControllerValue() != 64)
        {
            return error; // Inside the dead zone: still centered
        }
        ++sent;
    }
    (void) sent;

    out = run(engine, ev(ControlType::Axis, 1, 0.25f, 2 * ms));
    bool xMoved = false;
    for (MIDIEvent const &e : out)
    {
        if (e.getController() == 20 && e.getControllerValue() > 64)
        {
            xMoved = true; // The partner axis was re-evaluated when Y moved
        }
    }
    if (!xMoved)
    {
        return error;
    }

    // Full deflection reaches the full range.
    out = run(engine, ev(ControlType::Axis, 0, 1.0f, 3 * ms));
    out = run(engine, ev(ControlType::Axis, 1, 0.0f, 4 * ms));
    for (MIDIEvent const &e : out)
    {
        if (e.getController() == 20 && e.getControllerValue() != 127)
        {
            return error;
        }
    }
    return 0;
}

int main (int argc, char **argv)
{
    std::map<std::string, std::function<int()>> const tests = {
        { "test_status", test_status },
        { "test_channel", test_channel },
        { "test_note", test_note },
        { "test_velocity", test_velocity },
        { "test_polyphonic_aftertouch", test_polyphonic_aftertouch },
        { "test_controller", test_controller },
        { "test_controller_value", test_controller_value },
        { "test_program_change", test_program_change },
        { "test_channel_aftertouch", test_channel_aftertouch },
        { "test_pitch_bend", test_pitch_bend },
        { "test_dead_zone", test_dead_zone },
        { "test_curves", test_curves },
        { "test_quantize", test_quantize },
        { "test_axis_to_cc", test_axis_to_cc },
        { "test_invert_and_dead_zone", test_invert_and_dead_zone },
        { "test_unipolar_axis", test_unipolar_axis },
        { "test_change_suppression", test_change_suppression },
        { "test_rate_limit_and_flush", test_rate_limit_and_flush },
        { "test_smoothing", test_smoothing },
        { "test_pitch_bend_target", test_pitch_bend_target },
        { "test_button_note_tracking", test_button_note_tracking },
        { "test_release_on_remove", test_release_on_remove },
        { "test_hat_mappings", test_hat_mappings },
        { "test_pipeline_disconnect_releases_notes", test_pipeline_disconnect_releases_notes },
        { "test_profile_library_resolution", test_profile_library_resolution },
        { "test_default_profile_axes", test_default_profile_axes },
        { "test_shape_axis", test_shape_axis },
        { "test_x_and_y_use_different_ccs", test_x_and_y_use_different_ccs },
        { "test_radial_dead_zone", test_radial_dead_zone }
    };

    if (argc == 2)
    {
        auto it = tests.find(argv[1]);
        if (it == tests.end())
        {
            std::cerr << "Unknown test " << argv[1] << std::endl;
            return error;
        }
        return it->second();
    }

    return 0;
}
