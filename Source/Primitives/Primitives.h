#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Axis.h>
#include <Magnum/Primitives/Capsule.h>
#include <Magnum/Primitives/Circle.h>
#include <Magnum/Primitives/Cone.h>
#include <Magnum/Primitives/Crosshair.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Cylinder.h>
#include <Magnum/Primitives/Gradient.h>
#include <Magnum/Primitives/Grid.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Primitives/Line.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Trade/MeshData.h>

#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Shaders/VertexColorGL.h>

#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

#include "Application.h"

namespace Magnum
{

using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

using namespace Math::Literals;

class Plane : public Object3D, public SceneGraph::Drawable3D
{
  public:
    explicit Plane(Object3D &object, Shaders::PhongGL &shader, SceneGraph::DrawableGroup3D &drawables)
        : Object3D{&object}, SceneGraph::Drawable3D{*this, &drawables}, _shader{shader}
    {
        _mesh = MeshTools::compile(Primitives::grid3DSolid(Vector2i{10, 10}));
        _color = Color4{0.5f, 0.5f, 0.5f, 1.0f};

        rotateX(-90.0_degf).scale(Vector3{2, 2, 2});

        _id = Application::singleton()->_getUniqueID();
    }

  private:
    void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) override
    {
        _shader.setObjectId(_id)
            .setDiffuseColor(_color)
            .setAmbientColor(0x111111_rgbf)
            .setShininess(80.0f)
            .setLightPositions({Vector4{3.0f, 3.0f, 3.0f, 0.0f}})
            .setTransformationMatrix(transformationMatrix)
            .setNormalMatrix(transformationMatrix.normalMatrix())
            .setProjectionMatrix(camera.projectionMatrix())
            .draw(_mesh);
    }

    uint32_t _id;
    GL::Mesh _mesh;
    Shaders::PhongGL &_shader;
    Color4 _color; // Keep material props here in future
};

class Cube : public Object3D, public SceneGraph::Drawable3D
{
  public:
    explicit Cube(Object3D &object, Shaders::PhongGL &shader, SceneGraph::DrawableGroup3D &drawables)
        : Object3D{&object}, SceneGraph::Drawable3D{*this, &drawables}, _shader{shader}
    {
        _mesh = MeshTools::compile(Primitives::cubeSolid());
        _color = Color4{0.5f, 0.5f, 0.5f, 1.0f};
        _id = Application::singleton()->_getUniqueID();
    }

  private:
    void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) override
    {
        _shader.setObjectId(_id)
            .setDiffuseColor(_color)
            .setAmbientColor(0x111111_rgbf)
            .setShininess(80.0f)
            .setLightPositions({Vector4{3.0f, 3.0f, 3.0f, 0.0f}})
            .setTransformationMatrix(transformationMatrix)
            .setNormalMatrix(transformationMatrix.normalMatrix())
            .setProjectionMatrix(camera.projectionMatrix())
            .draw(_mesh);
    }

    uint32_t _id;
    GL::Mesh _mesh;
    Shaders::PhongGL &_shader;
    Color4 _color; // Keep material props here in future
};

class Sphere : public Object3D, public SceneGraph::Drawable3D
{
  public:
    explicit Sphere(Object3D &object, Shaders::PhongGL &shader, SceneGraph::DrawableGroup3D &drawables)
        : Object3D{&object}, SceneGraph::Drawable3D{*this, &drawables}, _shader{shader}
    {
        _mesh = MeshTools::compile(Primitives::icosphereSolid(3));
        _color = Color4{0.5f, 0.5f, 0.5f, 1.0f};
        _id = Application::singleton()->_getUniqueID();
    }

  private:
    void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) override
    {
        _shader.setObjectId(_id)
            .setDiffuseColor(_color)
            .setAmbientColor(0x111111_rgbf)
            .setShininess(80.0f)
            .setLightPositions({Vector4{3.0f, 3.0f, 3.0f, 0.0f}})
            .setTransformationMatrix(transformationMatrix)
            .setNormalMatrix(transformationMatrix.normalMatrix())
            .setProjectionMatrix(camera.projectionMatrix())
            .draw(_mesh);
    }

    uint32_t _id;
    GL::Mesh _mesh;
    Shaders::PhongGL &_shader;
    Color4 _color; // Keep material props here in future
};

class Cone : public Object3D, public SceneGraph::Drawable3D
{
  public:
    explicit Cone(Object3D &object, Shaders::PhongGL &shader, SceneGraph::DrawableGroup3D &drawables)
        : Object3D{&object}, SceneGraph::Drawable3D{*this, &drawables}, _shader{shader}
    {
        _mesh = MeshTools::compile(Primitives::coneSolid(10, 16, 1));
        _color = Color4{0.5f, 0.5f, 0.5f, 1.0f};
        _id = Application::singleton()->_getUniqueID();
    }

  private:
    void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) override
    {
        _shader.setObjectId(_id)
            .setDiffuseColor(_color)
            .setAmbientColor(0x111111_rgbf)
            .setShininess(80.0f)
            .setLightPositions({Vector4{3.0f, 3.0f, 3.0f, 0.0f}})
            .setTransformationMatrix(transformationMatrix)
            .setNormalMatrix(transformationMatrix.normalMatrix())
            .setProjectionMatrix(camera.projectionMatrix())
            .draw(_mesh);
    }

    uint32_t _id;
    GL::Mesh _mesh;
    Shaders::PhongGL &_shader;
    Color4 _color; // Keep material props here in future
};

class Capsule : public Object3D, public SceneGraph::Drawable3D
{
  public:
    explicit Capsule(Object3D &object, Shaders::PhongGL &shader, SceneGraph::DrawableGroup3D &drawables)
        : Object3D{&object}, SceneGraph::Drawable3D{*this, &drawables}, _shader{shader}
    {
        _mesh = MeshTools::compile(Primitives::capsule3DSolid(10, 10, 16, 0.5f));
        _color = Color4{0.5f, 0.5f, 0.5f, 1.0f};
        _id = Application::singleton()->_getUniqueID();
    }

  private:
    void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) override
    {
        _shader.setObjectId(_id)
            .setDiffuseColor(_color)
            .setAmbientColor(0x111111_rgbf)
            .setShininess(80.0f)
            .setLightPositions({Vector4{3.0f, 3.0f, 3.0f, 0.0f}})
            .setTransformationMatrix(transformationMatrix)
            .setNormalMatrix(transformationMatrix.normalMatrix())
            .setProjectionMatrix(camera.projectionMatrix())
            .draw(_mesh);
    }

    uint32_t _id;
    GL::Mesh _mesh;
    Shaders::PhongGL &_shader;
    Color4 _color; // Keep material props here in future
};

} // namespace Magnum