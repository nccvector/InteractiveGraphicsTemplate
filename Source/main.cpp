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
    void update() override;

    Object3D *tempObj;
};

MouseInteractionExample::MouseInteractionExample(const Arguments &arguments) : Application{arguments}
{
    std::vector<Object3D *> objs = {new Cube{_scene, _phongShader, _drawables},
                                    new Capsule{_scene, _phongShader, _drawables}};

    tempObj = objs[0];

    selectedObject = tempObj;
}

void MouseInteractionExample::update()
{
    // Do some processing
    float moveSpeed = 0.1f;
    auto mouseSensitivity = 0.003_radf;

    if (mouseOverViewport && Input::GetMouseButton(1))
    {
        // Update camera rotations
        _rotationPoint = mainCam->transformation().translation();

        // Rotation about world Y
        mainCam->transform(Matrix4::translation(_rotationPoint) *
                           Matrix4::rotationY(-mouseSensitivity * Input::GetMouseDelta().x()) *
                           Matrix4::translation(-_rotationPoint));

        // Rotation about local
        mainCam->transformLocal(Matrix4::rotationX(-mouseSensitivity * Input::GetMouseDelta().y()));

        // Move camera with keys
        if (Input::GetKey(KeyCode::LeftShift))
            moveSpeed *= 2;

        if (Input::GetKey(KeyCode::W))
            mainCam->translateLocal(VectorForward * -moveSpeed);

        if (Input::GetKey(KeyCode::A))
            mainCam->translateLocal(VectorLeft * moveSpeed);

        if (Input::GetKey(KeyCode::S))
            mainCam->translateLocal(VectorBackward * -moveSpeed);

        if (Input::GetKey(KeyCode::D))
            mainCam->translateLocal(VectorRight * moveSpeed);

        if (Input::GetKey(KeyCode::E))
            mainCam->translateLocal(VectorUp * moveSpeed);

        if (Input::GetKey(KeyCode::Q))
            mainCam->translateLocal(VectorDown * moveSpeed);
    }

    ImGui::ShowDemoWindow();

    // ImGui::Begin("Debug Size");
    // ImGui::SliderInt("size-x", &size[0], 10, 2000);
    // ImGui::SliderInt("size-y", &size[1], 10, 2000);
    // ImGui::End();
}

} // namespace Magnum

MAGNUM_APPLICATION_MAIN(Magnum::MouseInteractionExample)
