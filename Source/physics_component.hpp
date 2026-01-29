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
	float terminalVelocity{ 12.0f };
	float2 velocity{ 0.0f,0.0f };


	void DrawInInspector()override 
	{ 
		ImGui::Checkbox("Is Static", &Is_Static);
		ImGui::Checkbox("Affected By Gravity", &Affected_By_Gravity);
		ImGui::Checkbox("Grounded", &Is_Grounded);
		ImGui::DragFloat("Gravity", &gravity, 0.1f);
		ImGui::DragFloat2("Velocity", &velocity.x, 0.1f);
	} 
	void Serialize(Json::Value& outComp) const override
	{
		outComp["Is Static"] = Is_Static;
		outComp["Affected By Gravity"] = Affected_By_Gravity;
		outComp["Grounded"] = Is_Grounded;
		outComp["Gravity"] = gravity;
		outComp["Velocity"] = velocity.x;
	}

	void Deserialize(const Json::Value& compObj) override
	{
		if (compObj.isMember("Is Static") && compObj["Is Static"].isBool())
			Is_Static = compObj["Is Static"].asBool();
		if (compObj.isMember("Affected By Gravity") && compObj["Affected By Gravity"].isBool())
			Affected_By_Gravity = compObj["Affected By Gravity"].asBool();
		if (compObj.isMember("Grounded") && compObj["Grounded"].isBool())
			Is_Grounded = compObj["Grounded"].asBool();
		if (compObj.isMember("Gravity") && compObj["Gravity"].isNumeric())
			gravity = compObj["Gravity"].asFloat();
		if (compObj.isMember("Velocity") && compObj["Velocity"].isNumeric())
			velocity.x = compObj["Velocity"].asFloat();
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