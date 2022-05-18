#pragma once

#include "Application.h"
#include "Input.h"
#include "Layer.h"

#include "Toolbar.h"

class GUILayer : public Layer
{
  public:
    GUILayer(const char *name = "GUILayer") : Layer{name}
    {
    }

    void OnAttach() override
    {
        app = Application::singleton();
    }

    virtual void OnGUIRender() override
    {
        toolbar.Draw();

        if (app->selectedObject != nullptr)
        {
            ImGui::Begin("Transform");

            Magnum::Matrix4 matrix = app->selectedObject->transformation();

            if (ImGui::IsKeyPressed(90))
                app->mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(69))
                app->mCurrentGizmoOperation = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(82)) // r Key
                app->mCurrentGizmoOperation = ImGuizmo::SCALE;
            if (ImGui::RadioButton("Translate", app->mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
                app->mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::RadioButton("Rotate", app->mCurrentGizmoOperation == ImGuizmo::ROTATE))
                app->mCurrentGizmoOperation = ImGuizmo::ROTATE;
            ImGui::SameLine();
            if (ImGui::RadioButton("Scale", app->mCurrentGizmoOperation == ImGuizmo::SCALE))
                app->mCurrentGizmoOperation = ImGuizmo::SCALE;
            float matrixTranslation[3], matrixRotation[3], matrixScale[3];
            ImGuizmo::DecomposeMatrixToComponents(matrix.data(), matrixTranslation, matrixRotation, matrixScale);
            ImGui::InputFloat3("Translation", matrixTranslation, "%.2f");
            ImGui::InputFloat3("Rotation", matrixRotation, "%.2f");
            ImGui::InputFloat3("Scale", matrixScale, "%.2f");
            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix.data());

            if (app->mCurrentGizmoOperation != ImGuizmo::SCALE)
            {
                if (ImGui::RadioButton("Local", app->mCurrentGizmoMode == ImGuizmo::LOCAL))
                    app->mCurrentGizmoMode = ImGuizmo::LOCAL;
                ImGui::SameLine();
                if (ImGui::RadioButton("World", app->mCurrentGizmoMode == ImGuizmo::WORLD))
                    app->mCurrentGizmoMode = ImGuizmo::WORLD;
            }

            app->selectedObject->setTransformation(matrix);

            ImGui::End();
        }
    }

  private:
    Application *app;
    float menuBarHeight = 20.0f;
    float toolbarSize = 500.0f;

    Toolbar toolbar;
};