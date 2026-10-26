#include "Pipeline.h"

#include <future>

namespace
{

uint64_t const activityIntervalNs = 33000000; // ~30 Hz
int const pollTimeoutMs = 2;

} // namespace

Pipeline::Pipeline(std::unique_ptr<InputBackend> input, OutputFactory outputFactory, std::shared_ptr<ProfileLibrary> library) :
    m_input(std::move(input)),
    m_outputFactory(std::move(outputFactory)),
    m_library(std::move(library)),
    m_listener(nullptr),
    m_running(false)
{
}

Pipeline::~Pipeline()
{
    stop();
}

void Pipeline::setListener(PipelineListener *listener)
{
    m_listener = listener;
}

bool Pipeline::start(std::string &error)
{
    if (m_running)
    {
        return true;
    }

    // The backend is initialized on the worker thread that will also poll it.
    std::promise<std::string> initResult;
    std::future<std::string> initFuture = initResult.get_future();
    m_running = true;

    m_thread = std::thread([this, &initResult]() {
        std::string err;
        bool const ok = m_input->init(err);
        initResult.set_value(ok ? std::string() : (err.empty() ? std::string("init failed") : err));
        if (!ok)
        {
            return;
        }

        run();
        m_input->shutdown();
    });

    error = initFuture.get();
    if (!error.empty())
    {
        m_thread.join();
        m_running = false;
        return false;
    }

    return true;
}

void Pipeline::stop()
{
    if (!m_thread.joinable())
    {
        return;
    }

    m_running = false;
    m_thread.join();
}

void Pipeline::post(std::function<void()> command)
{
    std::lock_guard<std::mutex> lock(m_commandMutex);
    m_commands.push_back(std::move(command));
}

void Pipeline::setDeviceEnabled(DeviceId device, bool enabled)
{
    post([this, device, enabled]() {
        auto it = m_devices.find(device);
        if (it == m_devices.end())
        {
            return;
        }

        it->second.enabled = enabled;
        if (!enabled)
        {
            m_events.clear();
            m_engine.releaseAll(device, m_events);
            flushOutputs(m_events, device);
        }
    });
}

void Pipeline::reloadProfiles()
{
    post([this]() {
        for (auto &entry : m_devices)
        {
            std::string const name = applyProfile(entry.second);
            if (m_listener)
            {
                m_listener->onDeviceProfileChanged(entry.first, name);
            }
        }
    });
}

std::string Pipeline::applyProfile(Device &device)
{
    Profile const profile = m_library->resolve(device.info);

    m_events.clear();
    m_engine.setProfile(device.info.id, profile, m_events);
    flushOutputs(m_events, device.info.id);
    return profile.name;
}

void Pipeline::flushOutputs(MappingEngine::Events &events, DeviceId id)
{
    auto it = m_devices.find(id);
    if (it != m_devices.end() && it->second.output)
    {
        for (MIDIEvent const &e : events)
        {
            it->second.output->send(e);
        }
    }
    events.clear();
}

void Pipeline::run()
{
    while (m_running)
    {
        std::deque<std::function<void()>> commands;
        {
            std::lock_guard<std::mutex> lock(m_commandMutex);
            commands.swap(m_commands);
        }
        for (auto &c : commands)
        {
            c();
        }

        m_input->poll(pollTimeoutMs, *this);

        uint64_t const now = m_input->nowNs();
        for (auto &entry : m_devices)
        {
            if (entry.second.enabled && m_engine.hasPending(entry.first))
            {
                m_events.clear();
                m_engine.flush(entry.first, now, m_events);
                flushOutputs(m_events, entry.first);
            }
        }
    }

    // Leave no stuck notes behind.
    for (auto &entry : m_devices)
    {
        m_events.clear();
        m_engine.removeDevice(entry.first, m_events);
        flushOutputs(m_events, entry.first);
    }
    m_devices.clear();
}

void Pipeline::onDeviceAdded(DeviceInfo const &info)
{
    Device &device = m_devices[info.id];
    device.info = info;
    device.enabled = true;

    std::string error;
    device.output = m_outputFactory(info, error);
    if (!device.output && m_listener)
    {
        m_listener->onError("Cannot create MIDI output for " + info.name + ": " + error);
    }

    std::string const profileName = applyProfile(device);

    if (m_listener)
    {
        m_listener->onDeviceConnected(info, profileName);
    }
}

void Pipeline::onDeviceRemoved(DeviceId id)
{
    m_events.clear();
    m_engine.removeDevice(id, m_events);
    flushOutputs(m_events, id);
    m_devices.erase(id);

    if (m_listener)
    {
        m_listener->onDeviceDisconnected(id);
    }
}

void Pipeline::onControlEvent(ControlEvent const &event)
{
    auto it = m_devices.find(event.device);
    if (it == m_devices.end() || !it->second.enabled)
    {
        return;
    }

    m_events.clear();
    m_engine.processEvent(event, m_events);
    flushOutputs(m_events, event.device);

    Device &device = it->second;
    if (m_listener && event.timestamp - device.lastActivityNotify >= activityIntervalNs)
    {
        device.lastActivityNotify = event.timestamp;
        m_listener->onActivity(event.device);
    }
}
