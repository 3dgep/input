#include <input/Input.hpp>

#include <iostream>

using namespace input;

const char* keyNames[] = {
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "s",
    "t",
    "u",
    "v",
    "w",
    "x",
    "y",
    "z",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "up",
    "down",
    "left",
    "right",
    "[1]",
    "[2]",
    "[3]",
    "[4]",
    "[5]",
    "[6]",
    "[7]",
    "[8]",
    "[9]",
    "[0]",
    "[+]",
    "[-]",
    "[*]",
    "[=]",
    "[/]",
    "caps",
    "space",
    "delete",
    "ins",
    "home",
    "end",
    "enter",
    "esc",
    "tab",
    "backspace",
    "shift",
    "left shift",
    "right shift",
    "ctrl",
    "left ctrl",
    "right ctrl",
    "alt",
    "left alt",
    "right alt",
    "left win",
    "right win",
    "page up",
    "page down",
    ";",
    "+",
    ",",
    "-",
    ".",
    "?",
    "~",
    "[",
    "]",
    "|",
    "'",
    "f1",
    "f2",
    "f3",
    "f4",
    "f5",
    "f6",
    "f7",
    "f8",
    "f9",
    "f10",
    "f11",
    "f12"
};

const char* buttonNames[] = {
    "win",
    "mouse 0",
    "mouse 1",
    "mouse 2",
    "mouse x1",
    "mouse x2",
    "joystick button 1",
    "joystick button 2",
    "joystick button 3",
    "joystick button 4",
    "joystick button 5",
    "joystick button 6",
    "joystick button 7",
    "joystick button 8",
    "joystick button 9",
    "joystick button 10",
    "joystick dpad up",
    "joystick dpad down",
    "joystick dpad left",
    "joystick dpad right",
};

const char* axisNames[] = {
    "Horizontal",
    "Vertical",
    "Fire1",
    "Fire2",
    "Fire3",
    "Jump",
    "Submit",
    "Cancel",
    //"Mouse X", // Spammy
    //"Mouse Y", // Spammy
    //"Mouse ScrollWheel", // Spammy
};

void input_test()
{
    Input::update();  // Update keyboard, mouse and gamepad states.

    // Check keys
    for ( auto keyName: keyNames )
    {
        // Is the key pressed this frame?
        if ( Input::getKeyDown( keyName ) )
        {
            std::cout << keyName << " Pressed" << std::endl;
        }
        else if ( Input::getKey( keyName ) )
        {
            std::cout << keyName << std::endl;
        }
        else if ( Input::getKeyUp( keyName ) )
        {
            std::cout << keyName << " Released" << std::endl;
        }
    }

    // Check buttons
    for ( auto buttonName: buttonNames )
    {
        if ( Input::getButtonDown( buttonName ) )
        {
            std::cout << buttonName << " Pressed" << std::endl;
        }
        else if ( Input::getButton( buttonName ) )
        {
            std::cout << buttonName << std::endl;
        }
        else if ( Input::getButtonUp( buttonName ) )
        {
            std::cout << buttonName << " Released" << std::endl;
        }
    }

    // Check axis
    for ( auto axisName: axisNames )
    {
        float axis = Input::getAxis( axisName );
        if ( std::abs( axis ) > 0.0f )
        {
            std::cout << axisName << ": " << axis << std::endl;
        }
    }
}