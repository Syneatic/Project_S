#pragma once
#include <string>
#include "ImGUI/imgui.h"

#include "component.hpp"

//abstract
struct Renderer : Component
{
    //texture
    //color multiply
    //render method
    //render layer
    virtual void Draw()
    {

    }
};

struct SpriteRenderer : Renderer
{
    void Draw() override
    {
        //draw quad
        //renderSys::drawRect(float2);
        //put texture on
    }
    const std::string name() const override { return "SpriteRenderer"; }
};

struct MeshRenderer : Renderer
{
    const std::string name() const override { return "MeshRenderer"; }
};