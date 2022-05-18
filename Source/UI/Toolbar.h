#include "imgui_internal.h"

class Toolbar
{
  public:
    Toolbar()
    {
        app = Application::singleton();
    }

    void Draw()
    {
        ImVec2 topLeftMargin = {10, 33};
        ImVec2 viewportPos = app->viewportRectMin;
        ImVec2 viewportSize = ImVec2{app->viewportRectMax.x - viewportPos.x, app->viewportRectMax.y - viewportPos.y};

        ImGui::SetNextWindowPos(
            ImVec2{app->viewportRectMin.x + topLeftMargin.x, app->viewportRectMin.y + topLeftMargin.y});
        ImGui::SetNextWindowSize({buttonSize.x, viewportSize.y - topLeftMargin.y});

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoBackground;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoScrollbar;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Toolbar", nullptr, window_flags);

        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

        if (ImGui::Button("Plane", buttonSize))
        {
            Magnum::Debug{} << "CREATING PLANE";
        }

        if (ImGui::Button("Cube", buttonSize))
        {
            Magnum::Debug{} << "CREATING CUBE";
        }

        if (ImGui::Button("Sphere", buttonSize))
        {
            Magnum::Debug{} << "CREATING SPHERE";
        }

        if (ImGui::Button("Cone", buttonSize))
        {
            Magnum::Debug{} << "CREATING CONE";
        }

        if (ImGui::Button("Capsule", buttonSize))
        {
            Magnum::Debug{} << "CREATING CAPSULE";
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

  private:
    Application *app;
    ImVec2 buttonSize = {60, 60};
};