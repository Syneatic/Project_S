/*
Author: Jia Xi
Co-Author: Nil
*/
#pragma once

#include "camera.hpp"

struct MainCamera : Component
{
	// Camera behavior in scene
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	// Camera component data in editor
	void DrawInInspector() override;

	// Save / load camera component from file
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "Camera"; }

	MainCamera(GameObject& go) : Component(go) {};
	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};