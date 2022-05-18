#pragma once

#include <Magnum/Math/Matrix4.h>

#include <Magnum/GL/Texture.h>

#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Shaders/VertexColorGL.h>

#include <Magnum/Platform/GlfwApplication.h>

#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/ImGuiIntegration/Widgets.h>

#include "MainCamera.h"

#include "LayerStack.h"

#include <ImGuizmo.h>

#define VectorRight                                                                                                    \
    Magnum::Vector3                                                                                                    \
    {                                                                                                                  \
        1.0f, 0.0f, 0.0f                                                                                               \
    }

#define VectorLeft VectorRight * -1

#define VectorUp                                                                                                       \
    Magnum::Vector3                                                                                                    \
    {                                                                                                                  \
        0.0f, 1.0f, 0.0f                                                                                               \
    }

#define VectorDown VectorUp * -1

#define VectorForward                                                                                                  \
    Magnum::Vector3                                                                                                    \
    {                                                                                                                  \
        0.0f, 0.0f, 1.0f                                                                                               \
    }

#define VectorBackward VectorForward * -1

using Object3D = Magnum::SceneGraph::Object<Magnum::SceneGraph::MatrixTransformation3D>;
using Scene3D = Magnum::SceneGraph::Scene<Magnum::SceneGraph::MatrixTransformation3D>;

using namespace Magnum::Math::Literals;

class Application : public Magnum::Platform::Application
{
  public:
    explicit Application(const Arguments &arguments);

    static Application *singleton()
    {
        return instance;
    }

    bool EditTransform(Magnum::Matrix4 &matrix);
    void AddPlane();
    void AddCube();
    void AddSphere();
    void AddCone();
    void AddCapsule();

  private:
    void drawEvent() override;

    void viewportEvent(ViewportEvent &event) override;

    void keyPressEvent(KeyEvent &event) override;
    void keyReleaseEvent(KeyEvent &event) override;

    void mousePressEvent(MouseEvent &event) override;
    void mouseReleaseEvent(MouseEvent &event) override;
    void mouseMoveEvent(MouseMoveEvent &event) override;
    void mouseScrollEvent(MouseScrollEvent &event) override;
    void textInputEvent(TextInputEvent &event) override;

    // GUI Functions
    void _guiInit();
    void _guiBegin();
    void _guiEnd();
    void _guiDrawViewport();

  public:
    //================================================================================
    // GUILayer
    //================================================================================

    Magnum::Shaders::VertexColorGL3D _vertexColorShader{Corrade::NoCreate};
    Magnum::Shaders::FlatGL3D _flatShader{Corrade::NoCreate};
    Magnum::Shaders::PhongGL _phongShader{Corrade::NoCreate};
    Magnum::GL::Mesh _grid{Corrade::NoCreate};
    Magnum::Scene3D _scene;
    Magnum::SceneGraph::DrawableGroup3D _drawables;
    Magnum::SceneGraph::DrawableGroup3D _debugDrawables;
    Magnum::MainCamera *mainCam;

    int pMSAA = 8;
    bool mouseOverViewport = false;
    ImVec2 viewportRectMin;
    ImVec2 viewportRectMax;
    ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::LOCAL;

    Object3D *selectedObject = nullptr;

    // Display size
    Magnum::Vector2i size{500, 500};

    Magnum::GL::Texture2D *colorTexPtr;

    LayerStack layers;

  private:
    static Application *instance;

    Magnum::ImGuiIntegration::Context _imgui{Corrade::NoCreate};
    bool _showDemoWindow = true;
    bool _showAnotherWindow = false;
    Magnum::Color4 _clearColor = Magnum::Color4(0.1f, 0.1f, 0.1f, 1.0f);
    Magnum::Float _floatValue = 0.0f;
};
