#pragma once

#include <input/Input.hpp>

#include <GameInput.h>
#include <wrl.h>

#include <mutex>
#include <unordered_map>

class InputGDK
{
public:
    InputGDK();
    ~InputGDK();

    void addGamepad( IGameInputDevice* gamepad );
    void removeGamepad( IGameInputDevice* gamepad );

    static InputGDK& get();

private:
    Microsoft::WRL::ComPtr<IGameInput> m_GameInput;
    GameInputCallbackToken             m_DeviceCallbackToken;

    std::mutex                                                                        m_InputMutex;
    std::unordered_map<APP_LOCAL_DEVICE_ID, Microsoft::WRL::ComPtr<IGameInputDevice>> m_Gamepads;
    Microsoft::WRL::ComPtr<IGameInputDevice>                                          m_Keyboard;
    Microsoft::WRL::ComPtr<IGameInputDevice>                                          m_Mouse;
};