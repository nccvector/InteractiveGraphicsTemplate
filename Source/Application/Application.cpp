#include "Application.h"

#include <Magnum/Image.h>
#include <Magnum/ImageView.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/FunctionsBatch.h>

#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/TextureFormat.h>

#include <Magnum/Mesh.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Grid.h>

#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/TextureData.h>

#include "BasicDrawable.h"
#include "Input.h"
#include "Primitives.h"
#include <ImGuizmo.h>

using namespace Magnum;
using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

// Delaring singleton pointer
Application *Application::instance = nullptr;

Application::Application(const Arguments &arguments) : Platform::Application{arguments, NoCreate}
{
    /* Try 8x MSAA, fall back to zero samples if not possible. Enable only 2x
       MSAA if we have enough DPI. */
    {
        const Vector2 dpiScaling = this->dpiScaling({});
        Configuration conf;
        conf.setTitle("Venom Example").setSize(conf.size(), dpiScaling);
        conf.setWindowFlags(Configuration::WindowFlag::Resizable);

        GLConfiguration glConf;
        glConf.setSampleCount(dpiScaling.max() < 2.0f ? pMSAA : 2);
        if (!tryCreate(conf, glConf))
        {
            Debug{} << "MSAA falling back to 0";
            create(conf, glConf.setSampleCount(0));
            pMSAA = 0;
        }
    }

    /* Shaders, renderer setup */
    _vertexColorShader = Shaders::VertexColorGL3D{};
    _flatShader = Shaders::FlatGL3D{};
    _phongShader = Shaders::PhongGL{};
    _phongShader.setAmbientColor(0x747474_rgbf).setShininess(80.0f);

    /* Grid */
    _grid = MeshTools::compile(Primitives::grid3DWireframe({15, 15}));
    auto grid = new Object3D{&_scene};
    (*grid).rotateX(90.0_degf).scale(Vector3{8.0f});
    new FlatDrawable{*grid, _flatShader, _grid, _drawables};
    /* Grid */

    /* Set up the camera */
    mainCam = new MainCamera{_scene};

    // MUST INITIALIZE THE SINGLETON PTR
    // It is used by all other modules, better to initialize right after necessary app configurations
    Application::instance = this;

    // Initialize Input system
    Input::Init(window());

    // Initialize GUI
    _guiInit();

    // ADD LAYERS AT THE END...
}

void Application::update()
{
    // Implement the logic here...
}

void Application::drawEvent()
{
    // Input processing
    Input::update();

    // Layers::OnUpdate()
    for (auto layer : layers)
        layer->OnUpdate();

    //================================================================================

    // Prepare an 8x msaa buffer and render to it
    /* 8x MSAA */
    GL::Renderbuffer color, depthStencil;
    color.setStorageMultisample(pMSAA, GL::RenderbufferFormat::RGBA8, size);
    depthStencil.setStorageMultisample(pMSAA, GL::RenderbufferFormat::Depth24Stencil8, size);

    GL::Framebuffer framebufferMSAA{{{}, size}};
    framebufferMSAA.attachRenderbuffer(GL::Framebuffer::ColorAttachment{0}, color);
    framebufferMSAA.attachRenderbuffer(GL::Framebuffer::BufferAttachment::DepthStencil, depthStencil);

    framebufferMSAA.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth).bind();

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);

    mainCam->draw(_drawables);

    // Blit to a proxy buffer with texture
    // Display texture
    GL::Texture2D colorTex;
    colorTexPtr = &colorTex;
    colorTex.setStorage(1, GL::TextureFormat::RGBA8, size);

    GL::Framebuffer framebufferProxy{{{}, size}};
    framebufferProxy.attachTexture(GL::Framebuffer::ColorAttachment{0}, colorTex, 0);
    framebufferProxy.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth).bind();

    // Blitting to the default framebuffer
    GL::AbstractFramebuffer::blit(framebufferMSAA, framebufferProxy, framebufferMSAA.viewport(),
                                  GL::FramebufferBlit::Color);

    // // Blitting to the default framebuffer
    // GL::AbstractFramebuffer::blit(framebuffer, GL::defaultFramebuffer, framebuffer.viewport(),
    //                               GL::FramebufferBlit::Color);

    //================================================================================
    // Draw on default frame buffer
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color).bind();

    _guiBegin();

    // Layers::OnGUIRender()
    for (auto layer : layers)
        layer->OnGUIRender();

    _guiEnd();
    //================================================================================

    swapBuffers();
    redraw();
}

void Application::viewportEvent(ViewportEvent &event)
{
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    //================================================================================

    _imgui.relayout(Vector2{event.windowSize()} / event.dpiScaling(), event.windowSize(), event.framebufferSize());

    // Handle the event in layers
    for (auto layer : layers)
        layer->ViewportEvent(event);
}

void Application::keyPressEvent(KeyEvent &event)
{
    Input::updateDown(event.key());

    //================================================================================

    if (_imgui.handleKeyPressEvent(event))
        event.setAccepted(true);

    // Handle the event in layers
    for (auto layer : layers)
        layer->KeyPressEvent(event);
}

void Application::keyReleaseEvent(KeyEvent &event)
{
    Input::updateUp(event.key());

    //================================================================================

    if (_imgui.handleKeyReleaseEvent(event))
        event.setAccepted(true);

    // Handle the event in layers
    for (auto layer : layers)
        layer->KeyReleaseEvent(event);
}

void Application::mousePressEvent(MouseEvent &event)
{
    Input::updateMouseButtonDown(event.button());

    //================================================================================

    if (_imgui.handleMousePressEvent(event))
        return;

    // Handle the event in layers
    for (auto layer : layers)
        layer->MousePressEvent(event);
}

void Application::mouseReleaseEvent(MouseEvent &event)
{
    Input::updateMouseButtonUp(event.button());

    //================================================================================

    if (_imgui.handleMouseReleaseEvent(event))
        return;

    // Handle the event in layers
    for (auto layer : layers)
        layer->MouseReleaseEvent(event);
}

void Application::mouseMoveEvent(MouseMoveEvent &event)
{
    //================================================================================

    if (_imgui.handleMouseMoveEvent(event) != Application::singleton()->mouseOverViewport)
        return;

    // Handle the event in layers
    for (auto layer : layers)
        layer->MouseMoveEvent(event);
}

void Application::mouseScrollEvent(MouseScrollEvent &event)
{
    if (_imgui.handleMouseScrollEvent(event))
    {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }

    // Handle the event in layers
    for (auto layer : layers)
        layer->MouseScrollEvent(event);
}

void Application::textInputEvent(TextInputEvent &event)
{
    if (_imgui.handleTextInputEvent(event))
        event.setAccepted(true);

    // Handle the event in layers
    for (auto layer : layers)
        layer->TextInputEvent(event);
}

bool Application::EditTransform(Matrix4 &matrix)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

    ImGuiIO &io = ImGui::GetIO();

    ImGuizmo::SetRect(viewportRectMin.x, viewportRectMin.y, viewportRectMax.x - viewportRectMin.x,
                      viewportRectMax.y - viewportRectMin.y);

    ImGuizmo::Manipulate(mainCam->transformation().inverted().data(), mainCam->projectionMatrix().data(),
                         mCurrentGizmoOperation, mCurrentGizmoMode, matrix.data(), NULL, NULL);

    return ImGuizmo::IsUsing();
}

void Application::_guiInit()
{
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
}

void Application::_guiBegin()
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

    _guiDrawViewport();
}

void Application::_guiEnd()
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

void Application::_guiDrawViewport()
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