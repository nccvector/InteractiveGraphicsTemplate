#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Trade/MeshData.h>

#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Shaders/VertexColorGL.h>

#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

namespace Magnum
{

using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

using namespace Math::Literals;

class Cube : public Object3D, public SceneGraph::Drawable3D
{
  public:
    explicit Cube(Object3D &object, Shaders::PhongGL &shader, SceneGraph::DrawableGroup3D &drawables)
        : Object3D{&object}, SceneGraph::Drawable3D{*this, &drawables}, _shader{shader}
    {
        _mesh = MeshTools::compile(Primitives::cubeSolid());
        _color = Color4{0.5f, 0.5f, 0.5f, 1.0f};
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

    GL::Mesh _mesh;
    Shaders::PhongGL &_shader;
    Color4 _color; // Keep material props here in future
};

} // namespace Magnum