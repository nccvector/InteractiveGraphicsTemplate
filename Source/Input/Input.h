#include <Magnum/Platform/GlfwApplication.h>
#include <algorithm>
#include <vector>

namespace Input
{
bool keyDown[350];
bool keyHold[350];
bool keyUp[350];
std::vector<int> clearGroupDown;
std::vector<int> clearGroupUp;

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
}

void updateDown(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    if (keyHold[(int)key])
        return;

    keyDown[(int)key] = true;
    keyUp[(int)key] = false;
}

void updateUp(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    keyDown[(int)key] = false;
    keyUp[(int)key] = true;
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

    // Clearing groups
    clearGroupDown.clear();
    clearGroupUp.clear();

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
    }
}

bool GetKeyDown(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    return keyDown[(int)key];
}

bool GetKey(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    return keyDown[(int)key] | keyHold[(int)key];
}

bool GetKeyUp(Magnum::Platform::GlfwApplication::KeyEvent::Key key)
{
    return keyUp[(int)key];
}

} // namespace Input