#pragma once
#include "scene.hpp"

// ----------------------------------------------------------------
// Debugger overlay ¨C DEBUG builds only.
//
// Plugs into the existing ImGui frame that Scene::OnUpdate() owns.
// Does NOT start/end its own ImGui frame.
//
// Toggled by F5 via EngineCTX::debugMode.
// Call Debugger::Tick() INSIDE the existing ImGui frame block.
// ----------------------------------------------------------------
namespace Debugger
{
#ifdef _DEBUG
    // Call inside the existing #ifdef _DEBUG ImGui frame in Scene::OnUpdate().
    // Handles picking, outlines, hierarchy, inspector, error popup.
    void Tick(Scene& scene);

    // True when debugMode is on this frame ¡ª lets other systems block input.
    bool IsActive();

    // Push a non-fatal error into the overlay popup instead of crashing.
    void ReportError(const std::string& msg);
#else
    inline void Tick(Scene&) {}
    inline bool IsActive() { return false; }
    inline void ReportError(const std::string&) {}
#endif
}