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

#include "CameraControllerLayer.h"

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

    // Initialize other modules and layers
    Input::Init(window());

    gui.OnAttach();

    // ADD ALL THE LAYERS
    layers.PushLayer(new CameraControllerLayer());
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

    gui.Begin();

    // Layers::OnGUIRender()
    for (auto layer : layers)
        layer->OnGUIRender();

    //================================================================================

    gui.End();

    //================================================================================

    swapBuffers();
    redraw();
}

void Application::viewportEvent(ViewportEvent &event)
{
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    //================================================================================

    gui.ViewportEvent(event);

    // Handle the event in layers
    for (auto layer : layers)
        layer->ViewportEvent(event);
}

void Application::keyPressEvent(KeyEvent &event)
{
    Input::updateDown(event.key());

    //================================================================================

    gui.KeyPressEvent(event);

    // Handle the event in layers
    for (auto layer : layers)
        layer->KeyPressEvent(event);
}

void Application::keyReleaseEvent(KeyEvent &event)
{
    Input::updateUp(event.key());

    //================================================================================

    gui.KeyReleaseEvent(event);

    // Handle the event in layers
    for (auto layer : layers)
        layer->KeyReleaseEvent(event);
}

void Application::mousePressEvent(MouseEvent &event)
{
    Input::updateMouseButtonDown(event.button());

    //================================================================================

    gui.MousePressEvent(event);

    // Handle the event in layers
    for (auto layer : layers)
        layer->MousePressEvent(event);
}

void Application::mouseReleaseEvent(MouseEvent &event)
{
    Input::updateMouseButtonUp(event.button());

    //================================================================================

    gui.MouseReleaseEvent(event);

    // Handle the event in layers
    for (auto layer : layers)
        layer->MouseReleaseEvent(event);
}

void Application::mouseMoveEvent(MouseMoveEvent &event)
{
    //================================================================================

    gui.MouseMoveEvent(event);

    // Handle the event in layers
    for (auto layer : layers)
        layer->MouseMoveEvent(event);
}

void Application::mouseScrollEvent(MouseScrollEvent &event)
{
    gui.MouseScrollEvent(event);

    // Handle the event in layers
    for (auto layer : layers)
        layer->MouseScrollEvent(event);
}

void Application::textInputEvent(TextInputEvent &event)
{
    gui.TextInputEvent(event);

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