#pragma once

#include <Magnum/Platform/GlfwApplication.h>
#include <algorithm>
#include <vector>

namespace Input
{
extern bool keyDown[350];
extern bool keyHold[350];
extern bool keyUp[350];
extern bool mouseButtonDown[3];
extern bool mouseButtonHold[3];
extern bool mouseButtonUp[3];
extern Magnum::Vector2i _lastPosition;
extern Magnum::Vector2i mouseDelta;
extern Magnum::Vector2i mousePosition;

extern GLFWwindow *_window;

extern std::vector<int> clearGroupDown;
extern std::vector<int> clearGroupUp;
extern std::vector<int> clearGroupMouseButtonDown;
extern std::vector<int> clearGroupMouseButtonUp;

// PLEASE CALL INIT IN YOUR CONSTRUCTORS OR MAIN
void init(GLFWwindow *window);

void updateDown(Magnum::Platform::GlfwApplication::KeyEvent::Key key);

void updateMouseButtonDown(Magnum::Platform::GlfwApplication::MouseEvent::Button button);

void updateUp(Magnum::Platform::GlfwApplication::KeyEvent::Key key);

void updateMouseButtonUp(Magnum::Platform::GlfwApplication::MouseEvent::Button button);

void updateMouseMove(Magnum::Vector2i position);

// Call in update/drawEvent
void update();

bool GetKeyDown(Magnum::Platform::GlfwApplication::KeyEvent::Key key);

bool GetMouseButtonDown(int button);

bool GetMouseButtonDown(Magnum::Platform::GlfwApplication::MouseEvent::Button button);

bool GetKey(Magnum::Platform::GlfwApplication::KeyEvent::Key key);

bool GetMouseButton(int button);

bool GetMouseButton(Magnum::Platform::GlfwApplication::MouseEvent::Button button);

bool GetKeyUp(Magnum::Platform::GlfwApplication::KeyEvent::Key key);

bool GetMouseButtonUp(int button);

bool GetMouseButtonUp(Magnum::Platform::GlfwApplication::MouseEvent::Button button);

Magnum::Vector2i GetMouseDelta();

} // namespace Input

// Add this utility
using KeyCode = Magnum::Platform::GlfwApplication::KeyEvent::Key;