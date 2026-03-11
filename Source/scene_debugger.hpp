#pragma once
#include "scene.hpp"

// ----------------------------------------------------------------
// Debugger overlay ¨C DEBUG builds only.
//
// Wraps any running Scene. Physics, gameplay, audio all run
// normally. At the end of Scene::OnUpdate() call Debugger::Tick().
//
// Hover  -> cyan outline + inspector preview
// Click  -> locks inspector to that object (outline persists)
// Click empty space -> unlocks
// ----------------------------------------------------------------
namespace Debugger
{
#ifdef _DEBUG
    // Call once at the very end of Scene::OnUpdate().
    // Handles picking, outline drawing, and full ImGui overlay.
    void Tick(Scene& scene);
#else
    inline void Tick(Scene&) {}  // zero overhead in Release
#endif
}