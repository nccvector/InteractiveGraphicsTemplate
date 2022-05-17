#pragma once

#include <Magnum/Math/Matrix4.h>

#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/ImGuiIntegration/Widgets.h>

#include <Magnum/Platform/GlfwApplication.h>

#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Shaders/VertexColorGL.h>

#include "MainCamera.h"

#define VectorRight                                                                                                    \
    Vector3                                                                                                            \
    {                                                                                                                  \
        1.0f, 0.0f, 0.0f                                                                                               \
    }

#define VectorLeft VectorRight * -1

#define VectorUp                                                                                                       \
    Vector3                                                                                                            \
    {                                                                                                                  \
        0.0f, 1.0f, 0.0f                                                                                               \
    }

#define VectorDown VectorUp * -1

#define VectorForward                                                                                                  \
    Vector3                                                                                                            \
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

  private:
    virtual void update();

    void drawEvent() override;

    void viewportEvent(ViewportEvent &event) override;

    void keyPressEvent(KeyEvent &event) override;
    void keyReleaseEvent(KeyEvent &event) override;

    void mousePressEvent(MouseEvent &event) override;
    void mouseReleaseEvent(MouseEvent &event) override;
    void mouseMoveEvent(MouseMoveEvent &event) override;
    void mouseScrollEvent(MouseScrollEvent &event) override;
    void textInputEvent(TextInputEvent &event) override;

    bool EditTransform(Magnum::Matrix4 &matrix);

  public:
    //================================================================================
    Magnum::ImGuiIntegration::Context _imgui{Corrade::NoCreate};
    bool _showDemoWindow = true;
    bool _showAnotherWindow = false;
    Magnum::Color4 _clearColor = 0x72909aff_rgbaf;
    Magnum::Float _floatValue = 0.0f;
    //================================================================================

    Magnum::Shaders::VertexColorGL3D _vertexColorShader{Corrade::NoCreate};
    Magnum::Shaders::FlatGL3D _flatShader{Corrade::NoCreate};
    Magnum::Shaders::PhongGL _phongShader{Corrade::NoCreate};
    Magnum::GL::Mesh _grid{Corrade::NoCreate};
    Magnum::Scene3D _scene;
    Magnum::SceneGraph::DrawableGroup3D _drawables;
    Magnum::MainCamera *mainCam;

    Magnum::Float _lastDepth;
    Magnum::Vector2i _lastPosition{-1};
    Magnum::Vector3 _rotationPoint, _translationPoint;

    int pMSAA = 8;
    bool mouseOverViewport = false;
    ImVec2 viewportRectMin;
    ImVec2 viewportRectMax;

    Object3D *selectedObject;
};
