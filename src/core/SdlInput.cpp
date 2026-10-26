#include "SdlInput.h"

#include <SDL3/SDL.h>

#include <set>

namespace
{

float normalizeAxis(Sint16 v)
{
    return v < 0 ? static_cast<float>(v) / 32768.0f : static_cast<float>(v) / 32767.0f;
}

} // namespace

SdlInput::SdlInput() :
    m_initialized(false)
{
}

SdlInput::~SdlInput()
{
    shutdown();
}

bool SdlInput::init(std::string &error)
{
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    if (!SDL_Init(SDL_INIT_JOYSTICK))
    {
        error = SDL_GetError();
        return false;
    }

    m_initialized = true;
    return true;
}

void SdlInput::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    for (auto &entry : m_joysticks)
    {
        SDL_CloseJoystick(entry.second.joystick);
    }
    m_joysticks.clear();

    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    m_initialized = false;
}

uint64_t SdlInput::nowNs() const
{
    return SDL_GetTicksNS();
}

void SdlInput::handleAdded(DeviceId id, InputListener &listener)
{
    if (m_joysticks.count(id))
    {
        return;
    }

    SDL_Joystick *joystick = SDL_OpenJoystick(id);
    if (!joystick)
    {
        return;
    }

    char guidBuffer[64] = {0};
    SDL_GUIDToString(SDL_GetJoystickGUID(joystick), guidBuffer, sizeof(guidBuffer));

    DeviceInfo info;
    info.id = id;
    info.guid = guidBuffer;
    char const *name = SDL_GetJoystickName(joystick);
    info.name = name ? name : "Unknown joystick";
    char const *serial = SDL_GetJoystickSerial(joystick);
    info.serial = serial ? serial : "";
    info.numAxes = SDL_GetNumJoystickAxes(joystick);
    info.numButtons = SDL_GetNumJoystickButtons(joystick);
    info.numHats = SDL_GetNumJoystickHats(joystick);

    // Smallest free instance number among devices of the same type.
    std::set<int> used;
    for (auto const &entry : m_joysticks)
    {
        if (entry.second.guid == info.guid)
        {
            used.insert(entry.second.instance);
        }
    }
    int instance = 1;
    while (used.count(instance))
    {
        ++instance;
    }
    info.instance = instance;

    Opened opened = { joystick, info.guid, instance };
    m_joysticks[id] = opened;
    listener.onDeviceAdded(info);
}

void SdlInput::handleRemoved(DeviceId id, InputListener &listener)
{
    auto it = m_joysticks.find(id);
    if (it == m_joysticks.end())
    {
        return;
    }

    // Notify first so listeners can still release held notes.
    listener.onDeviceRemoved(id);
    SDL_CloseJoystick(it->second.joystick);
    m_joysticks.erase(it);
}

void SdlInput::poll(int timeoutMs, InputListener &listener)
{
    SDL_Event e;
    bool got = SDL_WaitEventTimeout(&e, timeoutMs);

    while (got)
    {
        ControlEvent ce;
        ce.timestamp = SDL_GetTicksNS();

        switch (e.type)
        {
        case SDL_EVENT_JOYSTICK_ADDED:
            handleAdded(e.jdevice.which, listener);
            break;
        case SDL_EVENT_JOYSTICK_REMOVED:
            handleRemoved(e.jdevice.which, listener);
            break;
        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
            ce.device = e.jaxis.which;
            ce.type = ControlType::Axis;
            ce.index = e.jaxis.axis;
            ce.value = normalizeAxis(e.jaxis.value);
            listener.onControlEvent(ce);
            break;
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
            ce.device = e.jbutton.which;
            ce.type = ControlType::Button;
            ce.index = e.jbutton.button;
            ce.value = e.jbutton.down ? 1.0f : 0.0f;
            listener.onControlEvent(ce);
            break;
        case SDL_EVENT_JOYSTICK_HAT_MOTION:
            ce.device = e.jhat.which;
            ce.type = ControlType::Hat;
            ce.index = e.jhat.hat;
            ce.value = static_cast<float>(e.jhat.value);
            listener.onControlEvent(ce);
            break;
        default:
            break;
        }

        got = SDL_PollEvent(&e);
    }
}
