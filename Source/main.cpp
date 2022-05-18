#include "Application.h"

#include "Primitives.h"

#include "Input.h"

namespace Magnum
{

using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

using namespace Math::Literals;

class MouseInteractionExample : public Application
{
  public:
    explicit MouseInteractionExample(const Arguments &arguments);

  private:
    Object3D *tempObj;
};

MouseInteractionExample::MouseInteractionExample(const Arguments &arguments) : Application{arguments}
{
    std::vector<Object3D *> objs = {new Cube{_scene, _phongShader, _drawables},
                                    new Capsule{_scene, _phongShader, _drawables}};

    tempObj = objs[0];

    // Set selected object
    selectedObject = tempObj;
}

} // namespace Magnum

MAGNUM_APPLICATION_MAIN(Magnum::MouseInteractionExample)
