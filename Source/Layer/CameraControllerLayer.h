#pragma once

#include "Application.h"
#include "Input.h"
#include "Layer.h"

class CameraControllerLayer : public Layer
{
  public:
    CameraControllerLayer(const char *name = "CameraControllerLayer") : Layer{name}
    {
    }

    void OnAttach() override
    {
        app = Application::singleton();
    }

    void OnDetach() override
    {
    }

    virtual void OnUpdate() override
    {
        // Do some processing
        float moveSpeed = 0.1f;
        auto mouseSensitivity = 0.003_radf;

        if (app->mouseOverViewport && Input::GetMouseButton(1))
        {
            // Update camera rotations
            Magnum::Vector3 _rotationPoint = app->mainCam->transformation().translation();

            // Rotation about world Y
            app->mainCam->transform(Magnum::Matrix4::translation(_rotationPoint) *
                                    Magnum::Matrix4::rotationY(-mouseSensitivity * Input::GetMouseDelta().x()) *
                                    Magnum::Matrix4::translation(-_rotationPoint));

            // Rotation about local
            app->mainCam->transformLocal(Magnum::Matrix4::rotationX(-mouseSensitivity * Input::GetMouseDelta().y()));

            // Move camera with keys
            if (Input::GetKey(KeyCode::LeftShift))
                moveSpeed *= 2;

            if (Input::GetKey(KeyCode::W))
                app->mainCam->translateLocal(VectorForward * -moveSpeed);

            if (Input::GetKey(KeyCode::A))
                app->mainCam->translateLocal(VectorLeft * moveSpeed);

            if (Input::GetKey(KeyCode::S))
                app->mainCam->translateLocal(VectorBackward * -moveSpeed);

            if (Input::GetKey(KeyCode::D))
                app->mainCam->translateLocal(VectorRight * moveSpeed);

            if (Input::GetKey(KeyCode::E))
                app->mainCam->translateLocal(VectorUp * moveSpeed);

            if (Input::GetKey(KeyCode::Q))
                app->mainCam->translateLocal(VectorDown * moveSpeed);
        }
    }

    virtual void OnGUIRender() override
    {
        ImGui::ShowDemoWindow();
    }

  private:
    Application *app;
};