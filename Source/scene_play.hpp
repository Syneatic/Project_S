#pragma once
#include "scene.hpp"

struct PlayScene : Scene
{
    void OnUpdate() override
    {
        AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);
        
        f32 dt = (f32)AEFrameRateControllerGetFrameTime();
        
        // Run physics
        Physics::Step(dt);
        
        // Update behaviours
        for (auto& pgo : _gameObjectList)
        {
            auto go = pgo.get();
            for (auto& [type, comp] : go->componentMap())
            {
                if (auto* b = dynamic_cast<Behaviour*>(comp.get()))
                    b->OnUpdate();
            }
        }
        
        // Render
        RenderSystem::Draw();
    }
    
    PlayScene() { _name = "Falling Object"; }
};