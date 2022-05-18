/*
DISCLAIMER
Taking inspiration from Hazel engine by "The Cherno".
The result is going to be not exactly the same, but the idea is
*/

#pragma once

#include <Magnum/Platform/GlfwApplication.h>

class Layer
{
  public:
    Layer(const char *name = "Layer")
    {
    }
    virtual ~Layer() = default;

    virtual void OnAttach()
    {
    }
    virtual void OnDetach()
    {
    }
    virtual void OnUpdate()
    {
    }
    virtual void OnGUIRender()
    {
    }

    virtual void ViewportEvent(Magnum::Platform::GlfwApplication::ViewportEvent &even)
    {
    }
    virtual void KeyPressEvent(Magnum::Platform::GlfwApplication::KeyEvent &event)
    {
    }
    virtual void KeyReleaseEvent(Magnum::Platform::GlfwApplication::KeyEvent &event)
    {
    }
    virtual void MousePressEvent(Magnum::Platform::GlfwApplication::MouseEvent &event)
    {
    }
    virtual void MouseReleaseEvent(Magnum::Platform::GlfwApplication::MouseEvent &event)
    {
    }
    virtual void MouseMoveEvent(Magnum::Platform::GlfwApplication::MouseMoveEvent &event)
    {
    }
    virtual void MouseScrollEvent(Magnum::Platform::GlfwApplication::MouseScrollEvent &event)
    {
    }
    virtual void TextInputEvent(Magnum::Platform::GlfwApplication::TextInputEvent &event)
    {
    }

  private:
    const char *mName;
};