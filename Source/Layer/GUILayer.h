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
    }

  private:
    Application *app;
    float menuBarHeight = 20.0f;
    float toolbarSize = 500.0f;

    Toolbar toolbar;
};