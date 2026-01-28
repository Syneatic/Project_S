#pragma once 
#include "component.hpp"
#include "math.hpp"
#include "physics.hpp"
#include "ImGUI/imgui.h"


struct RigidBody :Component 
{ 

	bool Affected_By_Gravity{ false };
	bool Is_Static{ false };
	bool Is_Grounded{ false };
	float gravity{ 9.8f };
	float2 velocity{ 0.0f,0.0f };
	void DrawInInspector()override 
	{ 
		ImGui::Checkbox("Is Static", &Is_Static);
		ImGui::Checkbox("Affected By Gravity", &Affected_By_Gravity);
		ImGui::Checkbox("Grounded", &Is_Grounded);
		ImGui::DragFloat("Gravity", &gravity, 0.1f);
		ImGui::DragFloat2("Velocity", &velocity.x, 0.1f);
	} 
	void Clear_Forces() 
	{ 
		if (Is_Static) 
		{ 
			velocity = float2::zero(); 
		} 
	} 
	const std::string name() const override { return "RigidBody"; }
};