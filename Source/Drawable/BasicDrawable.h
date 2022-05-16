#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Shaders/VertexColorGL.h>

namespace Magnum
{

using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

class ColoredDrawable : public SceneGraph::Drawable3D
{
  public:
    explicit ColoredDrawable(Object3D &object, Shaders::PhongGL &shader, GL::Mesh &mesh, const Color4 &color,
                             SceneGraph::DrawableGroup3D &group)
        : SceneGraph::Drawable3D{object, &group}, _shader(shader), _mesh(mesh), _color{color}
    {
    }

  private:
    void draw(const Matrix4 &transformationMatrix, SceneGraph::Camera3D &camera) override
    {
        _shader.setDiffuseColor(_color)
            .setAmbientColor(0x111111_rgbf)
            .setShininess(80.0f)
            .setLightPositions({Vector4{3.0f, 3.0f, 3.0f, 0.0f}})
            .setTransformationMatrix(transformationMatrix)
            .setNormalMatrix(transformationMatrix.normalMatrix())
            .setProjectionMatrix(camera.projectionMatrix())
            .draw(_mesh);
    }

    Shaders::PhongGL &_shader;
    GL::Mesh &_mesh;
    Color4 _color;
};

class VertexColorDrawable : public SceneGraph::Drawable3D
{
  public:
    explicit VertexColorDrawable(Object3D &object, Shaders::VertexColorGL3D &shader, GL::Mesh &mesh,
                                 SceneGraph::DrawableGroup3D &drawables)
        : SceneGraph::Drawable3D{object, &drawables}, _shader(shader), _mesh(mesh)
    {
    }

    void draw(const Matrix4 &transformation, SceneGraph::Camera3D &camera)
    {
        _shader.setTransformationProjectionMatrix(camera.projectionMatrix() * transformation).draw(_mesh);
    }

  private:
    Shaders::VertexColorGL3D &_shader;
    GL::Mesh &_mesh;
};

class FlatDrawable : public SceneGraph::Drawable3D
{
  public:
    explicit FlatDrawable(Object3D &object, Shaders::FlatGL3D &shader, GL::Mesh &mesh,
                          SceneGraph::DrawableGroup3D &drawables)
        : SceneGraph::Drawable3D{object, &drawables}, _shader(shader), _mesh(mesh)
    {
    }

    void draw(const Matrix4 &transformation, SceneGraph::Camera3D &camera)
    {
        _shader.setColor(0x747474_rgbf)
            .setTransformationProjectionMatrix(camera.projectionMatrix() * transformation)
            .draw(_mesh);
    }

  private:
    Shaders::FlatGL3D &_shader;
    GL::Mesh &_mesh;
};

} // namespace Magnum