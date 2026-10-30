#ifndef ODE2JOY_CORE_PIPELINE_H_
#define ODE2JOY_CORE_PIPELINE_H_

#include "InputBackend.h"
#include "MappingEngine.h"
#include "MidiOutput.h"
#include "ProfileLibrary.h"

#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

// Callbacks are invoked on the pipeline thread.
class PipelineListener
{
public:
    virtual ~PipelineListener() {}

    virtual void onDeviceConnected(DeviceInfo const &, std::string const & /*profileName*/) {}
    virtual void onDeviceDisconnected(DeviceId) {}
    virtual void onDeviceProfileChanged(DeviceId, std::string const & /*profileName*/) {}
    // Throttled; signals that a control of the device moved.
    virtual void onActivity(DeviceId) {}
    // Buttons and hats are reported on every change, axes are throttled per axis.
    virtual void onControlActivity(DeviceInfo const &, ControlEvent const &) {}
    virtual void onError(std::string const &) {}
};

// Runs input polling, mapping and MIDI output on one worker thread:
// InputBackend -> MappingEngine -> MidiOutput.
class Pipeline : private InputListener
{
public:
    typedef std::function<std::unique_ptr<MidiOutput>(DeviceInfo const &, std::string &)> OutputFactory;

    Pipeline(std::unique_ptr<InputBackend> input, OutputFactory outputFactory, std::shared_ptr<ProfileLibrary> library);
    ~Pipeline();

    // Fails if the input backend cannot be initialized.
    bool start(std::string &error);
    void stop();

    // Must be set before start().
    void setListener(PipelineListener *listener);

    // The following can be called from any thread and are executed asynchronously.
    void setDeviceEnabled(DeviceId device, bool enabled);
    // Re-resolves the profiles of all connected devices from the library.
    void reloadProfiles();

private:
    struct Device
    {
        DeviceInfo info;
        std::unique_ptr<MidiOutput> output;
        bool enabled = true;
        uint64_t lastActivityNotify = 0;
        std::map<int, uint64_t> lastAxisNotify;
    };

    void post(std::function<void()> command);
    void run();
    void flushOutputs(MappingEngine::Events &events, DeviceId device);
    std::string applyProfile(Device &device);

    void onDeviceAdded(DeviceInfo const &info) override;
    void onDeviceRemoved(DeviceId id) override;
    void onControlEvent(ControlEvent const &event) override;

    std::unique_ptr<InputBackend> m_input;
    OutputFactory m_outputFactory;
    std::shared_ptr<ProfileLibrary> m_library;
    PipelineListener *m_listener;

    std::thread m_thread;
    std::atomic<bool> m_running;
    std::mutex m_commandMutex;
    std::deque<std::function<void()>> m_commands;

    // Pipeline thread only
    MappingEngine m_engine;
    std::map<DeviceId, Device> m_devices;
    MappingEngine::Events m_events;
};

#endif
