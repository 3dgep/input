#include <input/Gamepad.hpp>

#include <GLFW/glfw3.h>

#include <stdexcept>

using namespace input;

const int       Gamepad::MAX_PLAYER_COUNT = 16;
constexpr float GLFWThumbDeadZone         = 0.24f;  // Consistent with XboxOneThumbDeadZone

class GamepadGLFW
{

public:
    static GamepadGLFW& get()
    {
        static GamepadGLFW instance;
        return instance;
    }

    void suspend()
    {
        
        m_Suspended = true;
    }

    void resume()
    {
        m_Suspended = false;
    }

    Gamepad::State getState( int player, Gamepad::DeadZone deadZoneMode )
    {
        Gamepad::State state = {};
        if ( m_Suspended )
            return state;

        if ( player == Gamepad::MOST_RECENT_PLAYER )
        {
            // Find most recent connected joystick
            for ( int i = 0; i < Gamepad::MAX_PLAYER_COUNT; ++i )
            {
                if ( glfwJoystickIsGamepad( GLFW_JOYSTICK_1 + i ) )
                {
                    player = i;
                }
            }
        }

        if ( player < 0 || player >= Gamepad::MAX_PLAYER_COUNT )
            return state;

        int jid = GLFW_JOYSTICK_1 + player;
        if ( !glfwJoystickIsGamepad( jid ) )
            return state;

        GLFWgamepadstate pad;
        if ( !glfwGetGamepadState( jid, &pad ) )
            return state;

        state.connected = true;
        state.packet    = glfwGetTimerValue();  // Use time as packet id (ms)

        // Buttons
        state.buttons.a             = pad.buttons[GLFW_GAMEPAD_BUTTON_A];
        state.buttons.b             = pad.buttons[GLFW_GAMEPAD_BUTTON_B];
        state.buttons.x             = pad.buttons[GLFW_GAMEPAD_BUTTON_X];
        state.buttons.y             = pad.buttons[GLFW_GAMEPAD_BUTTON_Y];
        state.buttons.leftStick     = pad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB];
        state.buttons.rightStick    = pad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB];
        state.buttons.leftShoulder  = pad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER];
        state.buttons.rightShoulder = pad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER];
        state.buttons.back          = pad.buttons[GLFW_GAMEPAD_BUTTON_BACK];
        state.buttons.start         = pad.buttons[GLFW_GAMEPAD_BUTTON_START];

        // DPad
        state.dPad.up    = pad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP];
        state.dPad.down  = pad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN];
        state.dPad.left  = pad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT];
        state.dPad.right = pad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT];

        // Raw thumbstick values
        float rawLeftX  = pad.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        float rawLeftY  = pad.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        float rawRightX = pad.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        float rawRightY = pad.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];

        // Apply deadzone
        ApplyStickDeadZone( rawLeftX, rawLeftY, deadZoneMode, 1.0f, GLFWThumbDeadZone, state.thumbSticks.leftX, state.thumbSticks.leftY );
        ApplyStickDeadZone( rawRightX, rawRightY, deadZoneMode, 1.0f, GLFWThumbDeadZone, state.thumbSticks.rightX, state.thumbSticks.rightY );

        // Triggers
        state.triggers.left  = pad.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] * 0.5f + 0.5f; // Map back in the range (0...1)
        state.triggers.right = pad.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] * 0.5f + 0.5f ; // Map back in the range (0...1)

        return state;
    }

    bool setVibration( int player, float leftMotor, float rightMotor, float leftTrigger, float rightTrigger )
    {
        // GLFW does not support vibration; stub implementation
        return false;
    }

private:
    GamepadGLFW()
    {
        if ( !glfwInit() )
        {
            throw std::runtime_error( "Failed to initialize GLFW" );
        }
    }

    ~GamepadGLFW()
    {
        // glfwTerminate(); // Uncomment if you want to terminate GLFW when this singleton is destroyed
    }

    bool m_Suspended = false;
};

Gamepad::State Gamepad::getState( DeadZone deadZoneMode ) const
{
    return GamepadGLFW::get().getState( playerIndex, deadZoneMode );
}

bool Gamepad::setVibration( float leftMotor, float rightMotor, float leftTrigger, float rightTrigger )
{
    return GamepadGLFW::get().setVibration( playerIndex, leftMotor, rightMotor, leftTrigger, rightTrigger );
}

void Gamepad::suspend() noexcept
{
    GamepadGLFW::get().suspend();
}

void Gamepad::resume() noexcept
{
    GamepadGLFW::get().resume();
}