#ifndef ODE2JOY_CORE_SDLINPUT_H_
#define ODE2JOY_CORE_SDLINPUT_H_

#include "InputBackend.h"

#include <map>

struct SDL_Joystick;

class SdlInput : public InputBackend
{
public:
    SdlInput();
    ~SdlInput() override;

    bool init(std::string &error) override;
    void shutdown() override;
    void poll(int timeoutMs, InputListener &listener) override;
    uint64_t nowNs() const override;

private:
    struct Opened
    {
        SDL_Joystick *joystick;
        std::string guid;
        int instance;
    };

    void handleAdded(DeviceId id, InputListener &listener);
    void handleRemoved(DeviceId id, InputListener &listener);

    std::map<DeviceId, Opened> m_joysticks;
    bool m_initialized;
};

#endif
