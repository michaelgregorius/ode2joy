#include "MidiOutput.h"

#include <RtMidi.h>

#include <vector>

namespace
{

class RtMidiOutput : public MidiOutput
{
public:
    explicit RtMidiOutput(std::string const &portName) :
        m_out(RtMidi::UNSPECIFIED, "ode2joy")
    {
        // Throws RtMidiError if virtual ports are unsupported (e.g. on Windows).
        m_out.openVirtualPort(portName);
    }

    void send(MIDIEvent const &event) override
    {
        m_message.clear();
        if (event.isSysEx())
        {
            m_message.assign(event.getSysExBytes(), event.getSysExBytes() + event.getSysExSize());
            try
            {
                m_out.sendMessage(&m_message);
            }
            catch (RtMidiError &)
            {
            }
            return;
        }

        m_message.push_back(event.getStatusByte());
        m_message.push_back(event.getData1Byte());

        MIDIEvent::Status const status = event.getStatus();
        if (status != MIDIEvent::ProgramChange && status != MIDIEvent::ChannelAftertouch)
        {
            m_message.push_back(event.getData2Byte());
        }

        try
        {
            m_out.sendMessage(&m_message);
        }
        catch (RtMidiError &)
        {
        }
    }

private:
    RtMidiOut m_out;
    std::vector<unsigned char> m_message;
};

} // namespace

std::unique_ptr<MidiOutput> createRtMidiOutput(DeviceInfo const &device, std::string &error)
{
    std::string portName = "ode2joy: " + device.name;
    if (device.instance > 1)
    {
        portName += " #" + std::to_string(device.instance);
    }

    try
    {
        return std::unique_ptr<MidiOutput>(new RtMidiOutput(portName));
    }
    catch (RtMidiError &e)
    {
        error = e.getMessage();
        return nullptr;
    }
}
