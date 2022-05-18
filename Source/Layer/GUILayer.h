#pragma once

#include "Layer.h"

#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/ImGuiIntegration/Widgets.h>

class GUILayer : public Layer
{
  public:
    GUILayer(const char *name = "GUILayer") : Layer{name}
    {
    }

    void OnAttach() override;

    void OnDetach() override;

    void Begin();

    void End();

    void DrawViewport();

    void ViewportEvent(Magnum::Platform::GlfwApplication::ViewportEvent &event) override;

    void KeyPressEvent(Magnum::Platform::GlfwApplication::KeyEvent &event) override;

    void KeyReleaseEvent(Magnum::Platform::GlfwApplication::KeyEvent &event) override;

    void MousePressEvent(Magnum::Platform::GlfwApplication::MouseEvent &event) override;

    void MouseReleaseEvent(Magnum::Platform::GlfwApplication::MouseEvent &event) override;

    void MouseMoveEvent(Magnum::Platform::GlfwApplication::MouseMoveEvent &event) override;

    void MouseScrollEvent(Magnum::Platform::GlfwApplication::MouseScrollEvent &event) override;

    void TextInputEvent(Magnum::Platform::GlfwApplication::TextInputEvent &event) override;

  private:
    Magnum::ImGuiIntegration::Context _imgui{Corrade::NoCreate};
    bool _showDemoWindow = true;
    bool _showAnotherWindow = false;
    Magnum::Color4 _clearColor = Magnum::Color4(0.1f, 0.1f, 0.1f, 1.0f);
    Magnum::Float _floatValue = 0.0f;
};