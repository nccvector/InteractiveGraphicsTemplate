#include "Input.h"

namespace Input
{
bool keyDown[350];
bool keyHold[350];
bool keyUp[350];
bool mouseButtonDown[3];
bool mouseButtonHold[3];
bool mouseButtonUp[3];
Magnum::Vector2i _lastPosition = Magnum::Vector2i{-1};
Magnum::Vector2i mouseDelta;
Magnum::Vector2i mousePosition;

std::vector<int> clearGroupDown;
std::vector<int> clearGroupUp;
std::vector<int> clearGroupMouseButtonDown;
std::vector<int> clearGroupMouseButtonUp;

// PLEASE CALL INIT IN YOUR CONSTRUCTORS OR MAIN
void init()
{
    // Initializing inputs
    for (int i = 0; i < 350; i++)
    {
        keyDown[i] = false;
        keyHold[i] = false;
        keyUp[i] = false;
    }

    for (int i = 0; i < 3; i++)
    {
        mouseButtonDown[i] = 0;
        mouseButtonHold[i] = 0;
        mouseButtonUp[i] = 0;
    }
}

void updateDown(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    if (keyHold[(int)key])
        return;

    keyDown[(int)key] = true;
    keyUp[(int)key] = false;
}

void updateMouseButtonDown(Magnum::Platform::GlfwApplication::MouseEvent::Button button)
{
    if (mouseButtonHold[(int)button])
        return;

    mouseButtonDown[(int)button] = true;
    mouseButtonUp[(int)button] = false;
}

void updateUp(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    keyDown[(int)key] = false;
    keyUp[(int)key] = true;
}

void updateMouseButtonUp(Magnum::Platform::GlfwApplication::MouseEvent::Button button)
{
    mouseButtonDown[(int)button] = false;
    mouseButtonUp[(int)button] = true;
}

void updateMouseMove(Magnum::Vector2i position)
{
    if (_lastPosition == Magnum::Vector2i{-1})
        _lastPosition = position;

    mousePosition = position;

    mouseDelta = mousePosition - _lastPosition;

    // Update last position
    _lastPosition = mousePosition;
}

// Call in update/drawEvent
void update()
{
    // Clear down keys from previos iteration
    for (int i : clearGroupDown)
        keyDown[i] = false;

    // Clear up keys from previous iteration
    for (int i : clearGroupUp)
        keyUp[i] = false;

    // Clear down keys from previos iteration
    for (int i : clearGroupMouseButtonDown)
        mouseButtonDown[i] = false;

    // Clear up keys from previous iteration
    for (int i : clearGroupMouseButtonUp)
        mouseButtonUp[i] = false;

    // Reset clear groups
    clearGroupDown.clear();
    clearGroupUp.clear();
    clearGroupMouseButtonDown.clear();
    clearGroupMouseButtonUp.clear();

    // Add to clear group if down
    for (int i = 0; i < 350; i++)
    {
        if (keyDown[i])
        {
            // Add to clear group so that the key can be reset
            clearGroupDown.push_back(i);

            // The key was down, so enable hold
            keyHold[i] = true;
        }
        else if (keyUp[i])
        {
            // Add to clear group for reset
            clearGroupUp.push_back(i);

            // The key is up, so disable hold
            keyHold[i] = false;
        }

        // Check for mouse buttons
        if (i < 3)
        {
            if (mouseButtonDown[i])
            {
                clearGroupMouseButtonDown.push_back(i);
                mouseButtonHold[i] = true;
            }
            else if (mouseButtonUp[i])
            {
                clearGroupMouseButtonUp.push_back(i);
                mouseButtonHold[i] = false;
            }
        }
    }
}

bool GetKeyDown(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    return keyDown[(int)key];
}

bool GetMouseButtonDown(int button)
{
    return mouseButtonDown[button];
}

bool GetMouseButtonDown(Magnum::Platform::GlfwApplication::MouseEvent::Button button)
{
    return mouseButtonDown[(int)button];
}

bool GetKey(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    return keyDown[(int)key] | keyHold[(int)key];
}

bool GetMouseButton(int button)
{
    return mouseButtonHold[button];
}

bool GetMouseButton(Magnum::Platform::GlfwApplication::MouseEvent::Button button)
{
    return mouseButtonHold[(int)button];
}

bool GetKeyUp(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    return keyUp[(int)key];
}

bool GetMouseButtonUp(int button)
{
    return mouseButtonUp[button];
}

bool GetMouseButtonUp(Magnum::Platform::GlfwApplication::MouseEvent::Button button)
{
    return mouseButtonUp[(int)button];
}

Magnum::Vector2i GetMouseDelta()
{
    return mouseDelta;
}

} // namespace Input