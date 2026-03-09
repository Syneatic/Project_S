#pragma once

#include "camera.hpp"

struct MainCamera : Component
{
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "Camera"; }
	GameObject* player = nullptr;

	MainCamera(GameObject& go) : Component(go) {};
	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};