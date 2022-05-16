#include <memory>

#include <Magnum/Image.h>
#include <Magnum/ImageView.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/FunctionsBatch.h>
#include <Magnum/Math/Matrix4.h>

#include <Magnum/Mesh.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Grid.h>

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

#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/ImGuiIntegration/Widgets.h>

#include <Magnum/MeshTools/Compile.h>

#include <Magnum/Platform/GlfwApplication.h>

#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Shaders/VertexColorGL.h>

#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/TextureData.h>

#include "BasicDrawable.h"
#include "MainCamera.h"
#include "Primitives.h"

namespace Magnum
{

using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

using namespace Math::Literals;

class MouseInteractionExample : public Platform::Application
{
  public:
    explicit MouseInteractionExample(const Arguments &arguments);

  private:
    Float depthAt(const Vector2i &windowPosition);
    Vector3 unproject(const Vector2i &windowPosition, Float depth) const;

    void drawEvent() override;

    void viewportEvent(ViewportEvent &event) override;

    void keyPressEvent(KeyEvent &event) override;
    void keyReleaseEvent(KeyEvent &event) override;

    void mousePressEvent(MouseEvent &event) override;
    void mouseReleaseEvent(MouseEvent &event) override;
    void mouseMoveEvent(MouseMoveEvent &event) override;
    void mouseScrollEvent(MouseScrollEvent &event) override;
    void textInputEvent(TextInputEvent &event) override;

    //================================================================================
    ImGuiIntegration::Context _imgui{NoCreate};
    bool _showDemoWindow = true;
    bool _showAnotherWindow = false;
    Color4 _clearColor = 0x72909aff_rgbaf;
    Float _floatValue = 0.0f;
    //================================================================================

    Shaders::VertexColorGL3D _vertexColorShader{NoCreate};
    Shaders::FlatGL3D _flatShader{NoCreate};
    Shaders::PhongGL _phongShader{NoCreate};
    GL::Mesh _mesh{NoCreate}, _grid{NoCreate};
    Scene3D _scene;
    SceneGraph::DrawableGroup3D _drawables;
    MainCamera *mainCam;

    Float _lastDepth;
    Vector2i _lastPosition{-1};
    Vector3 _rotationPoint, _translationPoint;

    int pMSAA = 8;
    bool mouseOverViewport = false;
};

MouseInteractionExample::MouseInteractionExample(const Arguments &arguments)
    : Platform::Application{arguments, NoCreate}
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

    // Creating a default cube
    Object3D *obj = new Cube{_scene, _phongShader, _drawables};
    obj->translate(Vector3{0.0f, 0.0f, -5.0f});

    /* Grid */
    _grid = MeshTools::compile(Primitives::grid3DWireframe({15, 15}));
    auto grid = new Object3D{&_scene};
    (*grid).rotateX(90.0_degf).scale(Vector3{8.0f});
    new FlatDrawable{*grid, _flatShader, _grid, _drawables};

    /* Set up the camera */
    mainCam = new MainCamera{_scene};

    /* Initialize initial depth to the value at scene center */
    _lastDepth = ((mainCam->projectionMatrix() * mainCam->cameraMatrix()).transformPoint({}).z() + 1.0f) * 0.5f;
}

void MouseInteractionExample::drawEvent()
{
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
        ImGui::ShowDemoWindow();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("OpenGL Viewport");
        ImGui::BeginChild("WinCont", ImVec2{0, 0}, false, ImGuiWindowFlags_NoMove);

        mouseOverViewport = ImGui::IsWindowHovered();
        if (mouseOverViewport)
        {
            // Get content rectangle
            ImVec2 vMin = ImGui::GetWindowContentRegionMin();
            ImVec2 vMax = ImGui::GetWindowContentRegionMax();

            vMin.x += ImGui::GetWindowPos().x;
            vMin.y += ImGui::GetWindowPos().y;
            vMax.x += ImGui::GetWindowPos().x;
            vMax.y += ImGui::GetWindowPos().y;

            ImVec2 screen_pos = ImGui::GetMousePos();

            if (screen_pos.x > vMin.x && screen_pos.y > vMin.y && screen_pos.x < vMax.x && screen_pos.y < vMax.y)
                mouseOverViewport = true;
            else
                mouseOverViewport = false;

            // Debug Rectangle
            ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(150, 150, 150, 255));
        }

        size = (Vector2i)(Vector2)ImGui::GetContentRegionMax();
        mainCam->setViewport(size);

        Magnum::ImGuiIntegration::image(colorTex, (Vector2)ImGui::GetContentRegionAvail());

        ImGui::EndChild();
        ImGui::End();
        ImGui::PopStyleVar();

        // ImGui::Begin("Debug Size");
        // ImGui::SliderInt("size-x", &size[0], 10, 2000);
        // ImGui::SliderInt("size-y", &size[1], 10, 2000);
        // ImGui::End();
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

    Debug{} << "Mouse over viewport" << mouseOverViewport;

    swapBuffers();
    redraw();
}

Float MouseInteractionExample::depthAt(const Vector2i &windowPosition)
{
    /* First scale the position from being relative to window size to being
       relative to framebuffer size as those two can be different on HiDPI
       systems */
    const Vector2i position = windowPosition * Vector2{framebufferSize()} / Vector2{windowSize()};
    const Vector2i fbPosition{position.x(), GL::defaultFramebuffer.viewport().sizeY() - position.y() - 1};

    GL::defaultFramebuffer.mapForRead(GL::DefaultFramebuffer::ReadAttachment::Front);
    Image2D data = GL::defaultFramebuffer.read(Range2Di::fromSize(fbPosition, Vector2i{1}).padded(Vector2i{2}),
                                               {GL::PixelFormat::DepthComponent, GL::PixelType::Float});

    /* TODO: change to just Math::min<Float>(data.pixels<Float>() when the
       batch functions in Math can handle 2D views */
    return Math::min<Float>(data.pixels<Float>().asContiguous());
}

Vector3 MouseInteractionExample::unproject(const Vector2i &windowPosition, Float depth) const
{
    /* We have to take window size, not framebuffer size, since the position is
       in window coordinates and the two can be different on HiDPI systems */
    const Vector2i viewSize = windowSize();
    const Vector2i viewPosition{windowPosition.x(), viewSize.y() - windowPosition.y() - 1};
    const Vector3 in{2 * Vector2{viewPosition} / Vector2{viewSize} - Vector2{1.0f}, depth * 2.0f - 1.0f};

    /*
        Use the following to get global coordinates instead of camera-relative:

        (mainCam->absoluteTransformationMatrix()*mainCam->projectionMatrix().inverted()).transformPoint(in)
    */
    return mainCam->projectionMatrix().inverted().transformPoint(in);
}

void MouseInteractionExample::viewportEvent(ViewportEvent &event)
{
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    _imgui.relayout(Vector2{event.windowSize()} / event.dpiScaling(), event.windowSize(), event.framebufferSize());
}

void MouseInteractionExample::keyPressEvent(KeyEvent &event)
{
    /* Reset the transformation to the original view */
    if (event.key() == KeyEvent::Key::Zero)
    {
        Debug{} << "ZERO PRESSED";
        mainCam->resetTransformation().translate(Vector3::zAxis(5.0f)).rotateX(-15.0_degf).rotateY(30.0_degf);
        redraw();
        return;

        /* Axis-aligned view */
    }
    else if (event.key() == KeyEvent::Key::One || event.key() == KeyEvent::Key::Three ||
             event.key() == KeyEvent::Key::Seven)
    {
        /* Start with current camera translation with the rotation inverted */
        const Vector3 viewTranslation =
            mainCam->transformation().rotationScaling().inverted() * mainCam->transformation().translation();

        /* Front/back */
        const Float multiplier = event.modifiers() & KeyEvent::Modifier::Ctrl ? -1.0f : 1.0f;

        Matrix4 transformation;
        if (event.key() == KeyEvent::Key::Seven) /* Top/bottom */
            transformation = Matrix4::rotationX(-90.0_degf * multiplier);
        else if (event.key() == KeyEvent::Key::One) /* Front/back */
            transformation = Matrix4::rotationY(90.0_degf - 90.0_degf * multiplier);
        else if (event.key() == KeyEvent::Key::Three) /* Right/left */
            transformation = Matrix4::rotationY(90.0_degf * multiplier);
        else
            CORRADE_INTERNAL_ASSERT_UNREACHABLE();

        mainCam->setTransformation(transformation * Matrix4::translation(viewTranslation));
        redraw();
    }

    //================================================================================

    if (_imgui.handleKeyPressEvent(event))
        return;
}

void MouseInteractionExample::keyReleaseEvent(KeyEvent &event)
{
    if (_imgui.handleKeyReleaseEvent(event))
        return;
}

void MouseInteractionExample::mousePressEvent(MouseEvent &event)
{
    /* Due to compatibility reasons, scroll is also reported as a press event,
       so filter that out. Could be removed once MouseEvent::Button::Wheel is
       gone from Magnum. */
    if (event.button() != MouseEvent::Button::Left && event.button() != MouseEvent::Button::Middle)
        return;

    const Float currentDepth = depthAt(event.position());
    const Float depth = currentDepth == 1.0f ? _lastDepth : currentDepth;
    _translationPoint = unproject(event.position(), depth);
    /* Update the rotation point only if we're not zooming against infinite
       depth or if the original rotation point is not yet initialized */
    if (currentDepth != 1.0f || _rotationPoint.isZero())
    {
        _rotationPoint = _translationPoint;
        _lastDepth = depth;
    }

    //================================================================================

    if (_imgui.handleMousePressEvent(event))
        return;
}

void MouseInteractionExample::mouseReleaseEvent(MouseEvent &event)
{
    if (_imgui.handleMouseReleaseEvent(event))
        return;
}

void MouseInteractionExample::mouseMoveEvent(MouseMoveEvent &event)
{
    if (_imgui.handleMouseMoveEvent(event) != mouseOverViewport)
        return;

    //================================================================================

    if (_lastPosition == Vector2i{-1})
        _lastPosition = event.position();
    const Vector2i delta = event.position() - _lastPosition;
    _lastPosition = event.position();

    if (!event.buttons())
        return;

    /* Translate */
    if (event.modifiers() & MouseMoveEvent::Modifier::Shift)
    {
        const Vector3 p = unproject(event.position(), _lastDepth);
        mainCam->translateLocal(_translationPoint - p); /* is Z always 0? */
        _translationPoint = p;

        /* Rotate around rotation point */
    }
    else
        mainCam->transformLocal(Matrix4::translation(_rotationPoint) * Matrix4::rotationX(-0.01_radf * delta.y()) *
                                Matrix4::rotationY(-0.01_radf * delta.x()) * Matrix4::translation(-_rotationPoint));

    redraw();
}

void MouseInteractionExample::mouseScrollEvent(MouseScrollEvent &event)
{
    const Float currentDepth = depthAt(event.position());
    const Float depth = currentDepth == 1.0f ? _lastDepth : currentDepth;
    const Vector3 p = unproject(event.position(), depth);
    /* Update the rotation point only if we're not zooming against infinite
       depth or if the original rotation point is not yet initialized */
    if (currentDepth != 1.0f || _rotationPoint.isZero())
    {
        _rotationPoint = p;
        _lastDepth = depth;
    }

    const Float direction = event.offset().y();
    if (!direction)
        return;

    /* Move towards/backwards the rotation point in cam coords */
    mainCam->translateLocal(_rotationPoint * direction * 0.1f);

    event.setAccepted();
    redraw();

    //================================================================================

    if (_imgui.handleMouseScrollEvent(event))
    {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void MouseInteractionExample::textInputEvent(TextInputEvent &event)
{
    if (_imgui.handleTextInputEvent(event))
        return;
}

} // namespace Magnum

MAGNUM_APPLICATION_MAIN(Magnum::MouseInteractionExample)
