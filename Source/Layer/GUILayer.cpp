#include "GUILayer.h"

#include "Application.h"

#include <Magnum/GL/Renderer.h>

#include <ImGuizmo.h>

using namespace Magnum;

void GUILayer::OnAttach()
{
    //================================================================================
    _imgui = Magnum::ImGuiIntegration::Context(
        Vector2{Application::singleton()->windowSize()} / Application::singleton()->dpiScaling(),
        Application::singleton()->windowSize(), Application::singleton()->framebufferSize());

    // Setup Dear ImGui context
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    /* Set up proper blending to be used by ImGui. There's a great chance
       you'll need this exact behavior for the rest of your scene. If not, set
       this only for the drawFrame() call. */
    Magnum::GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);
    Magnum::GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                           GL::Renderer::BlendFunction::OneMinusSourceAlpha);
    //================================================================================
}

void GUILayer::OnDetach()
{
}

void GUILayer::Begin()
{
    _imgui.newFrame();

    /* Check for text input */
    if (ImGui::GetIO().WantTextInput && !Application::singleton()->isTextInputActive())
        Application::singleton()->startTextInput();
    else if (!ImGui::GetIO().WantTextInput && Application::singleton()->isTextInputActive())
        Application::singleton()->stopTextInput();

    // START IMGUIZMO
    ImGuizmo::BeginFrame();

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    if (true)
        window_flags |= ImGuiWindowFlags_MenuBar;

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Venom Example", nullptr, window_flags);
    ImGui::PopStyleVar();

    ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("VenomDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    //================================================================================

    DrawViewport();
}

void GUILayer::End()
{
    ImGui::End();

    /* Update application cursor */
    _imgui.updateApplicationCursor(*Application::singleton());

    /* Set appropriate states. If you only draw ImGui, it is sufficient to
       just enable blending and scissor test in the constructor. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

    _imgui.drawFrame();
}

void GUILayer::DrawViewport()
{
    // Drawing Viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("OpenGL Viewport");
    ImGui::BeginChild("WinCont", ImVec2{0, 0}, false, ImGuiWindowFlags_NoMove);

    // UPDATE VIEWPORT RECT
    Application::singleton()->viewportRectMin = ImGui::GetWindowContentRegionMin();
    Application::singleton()->viewportRectMax = ImGui::GetWindowContentRegionMax();
    Application::singleton()->viewportRectMin.x += ImGui::GetWindowPos().x;
    Application::singleton()->viewportRectMin.y += ImGui::GetWindowPos().y;
    Application::singleton()->viewportRectMax.x += ImGui::GetWindowPos().x;
    Application::singleton()->viewportRectMax.y += ImGui::GetWindowPos().y;

    Application::singleton()->mouseOverViewport = ImGui::IsWindowHovered();
    if (Application::singleton()->mouseOverViewport)
    {

        ImVec2 screen_pos = ImGui::GetMousePos();

        if (screen_pos.x > Application::singleton()->viewportRectMin.x &&
            screen_pos.y > Application::singleton()->viewportRectMin.y &&
            screen_pos.x < Application::singleton()->viewportRectMax.x &&
            screen_pos.y < Application::singleton()->viewportRectMax.y)
            Application::singleton()->mouseOverViewport = true;
        else
            Application::singleton()->mouseOverViewport = false;

        // // Debug Rectangle
        // ImGui::GetForegroundDrawList()->AddRect(Application::singleton()->viewportRectMin,
        // Application::singleton()->viewportRectMax, IM_COL32(150, 150, 150, 255));
    }

    Application::singleton()->size = (Vector2i)(Vector2)ImGui::GetContentRegionMax();
    Application::singleton()->mainCam->setViewport(Application::singleton()->size);

    Magnum::ImGuiIntegration::image(*Application::singleton()->colorTexPtr, (Vector2)ImGui::GetContentRegionAvail());

    // Transform handling (IMGUIZMO DRAWING)
    Matrix4 mat = Application::singleton()->selectedObject->transformation();
    if (Application::singleton()->EditTransform(mat))
        Application::singleton()->selectedObject->setTransformation(mat);

    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar();
}

void GUILayer::ViewportEvent(Magnum::Platform::GlfwApplication::ViewportEvent &event)
{
    _imgui.relayout(Vector2{event.windowSize()} / event.dpiScaling(), event.windowSize(), event.framebufferSize());
}

void GUILayer::KeyPressEvent(Magnum::Platform::GlfwApplication::KeyEvent &event)
{
    if (_imgui.handleKeyPressEvent(event))
        event.setAccepted(true);
}

void GUILayer::KeyReleaseEvent(Magnum::Platform::GlfwApplication::KeyEvent &event)
{
    if (_imgui.handleKeyReleaseEvent(event))
        event.setAccepted(true);
}

void GUILayer::MousePressEvent(Magnum::Platform::GlfwApplication::MouseEvent &event)
{
    if (_imgui.handleMousePressEvent(event))
        return;
}

void GUILayer::MouseReleaseEvent(Magnum::Platform::GlfwApplication::MouseEvent &event)
{
    if (_imgui.handleMouseReleaseEvent(event))
        return;
}

void GUILayer::MouseMoveEvent(Magnum::Platform::GlfwApplication::MouseMoveEvent &event)
{
    if (_imgui.handleMouseMoveEvent(event) != Application::singleton()->mouseOverViewport)
        return;
}

void GUILayer::MouseScrollEvent(Magnum::Platform::GlfwApplication::MouseScrollEvent &event)
{
    if (_imgui.handleMouseScrollEvent(event))
    {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void GUILayer::TextInputEvent(Magnum::Platform::GlfwApplication::TextInputEvent &event)
{
    if (_imgui.handleTextInputEvent(event))
        event.setAccepted(true);
}