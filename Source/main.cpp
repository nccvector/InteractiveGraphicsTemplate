#include "Application.h"
#include "Primitives.h"

#include "CameraControllerLayer.h"
#include "GUILayer.h"

namespace Magnum
{

using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

using namespace Math::Literals;

class MyApplication : public Application
{
  public:
    explicit MyApplication(const Arguments &arguments);

  private:
};

MyApplication::MyApplication(const Arguments &arguments) : Application{arguments}
{
    // Create some objects
    std::vector<Object3D *> objs = {new Cube{_scene, _phongShader, _drawables},
                                    new Capsule{_scene, _phongShader, _drawables}};

    // Set selected object
    selectedObject = objs[0];

    // ADD ALL THE LAYERS
    layers.PushLayer(new CameraControllerLayer());
    layers.PushLayer(new GUILayer());
}

} // namespace Magnum

MAGNUM_APPLICATION_MAIN(Magnum::MyApplication)
