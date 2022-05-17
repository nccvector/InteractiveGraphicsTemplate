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
#include <Magnum/GL/Texture.h>
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

    //================================================================================
    _imgui = ImGuiIntegration::Context(Vector2{windowSize()} / dpiScaling(), windowSize(), framebufferSize());

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
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                   GL::Renderer::BlendFunction::OneMinusSourceAlpha);
    //================================================================================

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

    /* Set up the camera */
    mainCam = new MainCamera{_scene};

    /* Initialize initial depth to the value at scene center */
    _lastDepth = ((mainCam->projectionMatrix() * mainCam->cameraMatrix()).transformPoint({}).z() + 1.0f) * 0.5f;

    Input::init();
}

void Application::update()
{
}

void Application::drawEvent()
{
    // Input processing
    Input::update();

    double x, y;
    glfwGetCursorPos(window(), &x, &y);
    Input::updateMouseMove(Vector2i{(int)x, (int)y});

    //================================================================================

    // Display size
    static Vector2i size{500, 500};

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

    _imgui.newFrame();

    /* Check for text input */
    if (ImGui::GetIO().WantTextInput && !isTextInputActive())
        startTextInput();
    else if (!ImGui::GetIO().WantTextInput && isTextInputActive())
        stopTextInput();

    ImGuizmo::BeginFrame();

    {
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
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
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

        // Do your update here...
        update();

        // Drawing Viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("OpenGL Viewport");
        ImGui::BeginChild("WinCont", ImVec2{0, 0}, false, ImGuiWindowFlags_NoMove);

        // UPDATE VIEWPORT RECT
        viewportRectMin = ImGui::GetWindowContentRegionMin();
        viewportRectMax = ImGui::GetWindowContentRegionMax();
        viewportRectMin.x += ImGui::GetWindowPos().x;
        viewportRectMin.y += ImGui::GetWindowPos().y;
        viewportRectMax.x += ImGui::GetWindowPos().x;
        viewportRectMax.y += ImGui::GetWindowPos().y;

        mouseOverViewport = ImGui::IsWindowHovered();
        if (mouseOverViewport)
        {

            ImVec2 screen_pos = ImGui::GetMousePos();

            if (screen_pos.x > viewportRectMin.x && screen_pos.y > viewportRectMin.y &&
                screen_pos.x < viewportRectMax.x && screen_pos.y < viewportRectMax.y)
                mouseOverViewport = true;
            else
                mouseOverViewport = false;

            // // Debug Rectangle
            // ImGui::GetForegroundDrawList()->AddRect(viewportRectMin, viewportRectMax, IM_COL32(150, 150, 150, 255));
        }

        size = (Vector2i)(Vector2)ImGui::GetContentRegionMax();
        mainCam->setViewport(size);

        Magnum::ImGuiIntegration::image(colorTex, (Vector2)ImGui::GetContentRegionAvail());

        // Transform handling (IMGUIZMO DRAWING)
        Matrix4 mat = selectedObject->transformation();
        if (EditTransform(mat))
            selectedObject->setTransformation(mat);

        ImGui::EndChild();
        ImGui::End();
        ImGui::PopStyleVar();

        //================================================================================

        ImGui::End();
    }

    /* Update application cursor */
    _imgui.updateApplicationCursor(*this);

    /* Set appropriate states. If you only draw ImGui, it is sufficient to
       just enable blending and scissor test in the constructor. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

    _imgui.drawFrame();

    //================================================================================

    swapBuffers();
    redraw();
}

void Application::viewportEvent(ViewportEvent &event)
{
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    _imgui.relayout(Vector2{event.windowSize()} / event.dpiScaling(), event.windowSize(), event.framebufferSize());
}

void Application::keyPressEvent(KeyEvent &event)
{
    Input::updateDown(event.key());

    //================================================================================

    if (_imgui.handleKeyPressEvent(event))
        event.setAccepted(true);
}

void Application::keyReleaseEvent(KeyEvent &event)
{
    Input::updateUp(event.key());

    if (_imgui.handleKeyReleaseEvent(event))
        event.setAccepted(true);
}

void Application::mousePressEvent(MouseEvent &event)
{
    Input::updateMouseButtonDown(event.button());

    //================================================================================

    if (_imgui.handleMousePressEvent(event))
        return;
}

void Application::mouseReleaseEvent(MouseEvent &event)
{
    Input::updateMouseButtonUp(event.button());

    if (_imgui.handleMouseReleaseEvent(event))
        return;
}

void Application::mouseMoveEvent(MouseMoveEvent &event)
{
    //================================================================================

    if (_imgui.handleMouseMoveEvent(event) != mouseOverViewport)
        return;
}

void Application::mouseScrollEvent(MouseScrollEvent &event)
{
    if (_imgui.handleMouseScrollEvent(event))
    {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void Application::textInputEvent(TextInputEvent &event)
{
    if (_imgui.handleTextInputEvent(event))
        event.setAccepted(true);
}

bool Application::EditTransform(Matrix4 &matrix)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

    // if (ImGui::IsKeyPressed(90))
    //     mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    // if (ImGui::IsKeyPressed(69))
    //     mCurrentGizmoOperation = ImGuizmo::ROTATE;
    // if (ImGui::IsKeyPressed(82)) // r Key
    //     mCurrentGizmoOperation = ImGuizmo::SCALE;
    // if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
    //     mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    // ImGui::SameLine();
    // if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
    //     mCurrentGizmoOperation = ImGuizmo::ROTATE;
    // ImGui::SameLine();
    // if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
    //     mCurrentGizmoOperation = ImGuizmo::SCALE;

    // float matrixTranslation[3];
    // float matrixRotation[3];
    // float matrixScale[3];

    // ImGuizmo::DecomposeMatrixToComponents(matrix.data(), matrixTranslation, matrixRotation, matrixScale);
    // ImGui::InputFloat3("Tr", matrixTranslation, "%.3f");
    // ImGui::InputFloat3("Rt", matrixRotation, "%.3f");
    // ImGui::InputFloat3("Sc", matrixScale, "%.3f");
    // ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix.data());

    // if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    // {
    //     if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
    //         mCurrentGizmoMode = ImGuizmo::LOCAL;
    //     ImGui::SameLine();
    //     if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
    //         mCurrentGizmoMode = ImGuizmo::WORLD;
    // }

    // static bool useSnap(false);
    // if (ImGui::IsKeyPressed(83))
    //     useSnap = !useSnap;
    // ImGui::Checkbox("", &useSnap);
    // ImGui::SameLine();
    // vec_t snap;
    // switch (mCurrentGizmoOperation)
    // {
    // case ImGuizmo::TRANSLATE:
    //     snap = config.mSnapTranslation;
    //     ImGui::InputFloat3("Snap", &snap.x);
    //     break;
    // case ImGuizmo::ROTATE:
    //     snap = config.mSnapRotation;
    //     ImGui::InputFloat("Angle Snap", &snap.x);
    //     break;
    // case ImGuizmo::SCALE:
    //     snap = config.mSnapScale;
    //     ImGui::InputFloat("Scale Snap", &snap.x);
    //     break;
    // }

    ImGuiIO &io = ImGui::GetIO();

    ImGuizmo::SetRect(viewportRectMin.x, viewportRectMin.y, viewportRectMax.x - viewportRectMin.x,
                      viewportRectMax.y - viewportRectMin.y);

    ImGuizmo::Manipulate(mainCam->transformation().inverted().data(), mainCam->projectionMatrix().data(),
                         mCurrentGizmoOperation, mCurrentGizmoMode, matrix.data(), NULL, NULL);

    return ImGuizmo::IsUsing();
}