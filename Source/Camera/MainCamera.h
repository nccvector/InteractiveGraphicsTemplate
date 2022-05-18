#pragma once

#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

#include <Magnum/GL/DefaultFramebuffer.h>

namespace Magnum
{

using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

using namespace Math::Literals;

class MainCamera : public Object3D, public SceneGraph::Camera3D
{
  public:
    explicit MainCamera(Object3D &object) : SceneGraph::Camera3D{*this}, Object3D{&object}
    {
        setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend);
        setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 1000.0f));
        setViewport(GL::defaultFramebuffer.viewport().size());
        translate(Vector3::zAxis(5.0f)).rotateX(-15.0_degf).rotateY(30.0_degf);
    }
};

} // namespace Magnum