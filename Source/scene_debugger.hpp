/*
Author: Muhammad Harith Bin Khairudyn
Co-Author: NIL
*/
#pragma once
#include "scene.hpp"

// ----------------------------------------------------------------
// Debugger overlay ¨C DEBUG builds only.
//
// Plugs into the existing ImGui frame that Scene::OnUpdate() owns.
// Does NOT start/end its own ImGui frame.
//
// Toggled by F5 via EngineCTX::debugMode.
// ----------------------------------------------------------------
namespace Debugger
{
#ifdef _DEBUG
    void Tick(Scene& scene);
    bool IsActive();
    void ReportError(const std::string& msg);
    void Reset();          // <-- ADD: call on scene exit to clear stale pointers
#else
    inline void Tick(Scene&) {}
    inline bool IsActive() { return false; }
    inline void ReportError(const std::string&) {}
    inline void Reset() {}
#endif
}