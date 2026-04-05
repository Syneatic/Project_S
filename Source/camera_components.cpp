/*
Author: Jia Xi
Co-Author: Nil
*/
#include "camera_components.hpp"
#include "gameobject.hpp"

// Camera behavior in scene
void MainCamera::OnStart()
{
	CameraSystem::OnStart();
};
void MainCamera::OnUpdate() {
	CameraSystem::MoveCamera(_owner.transform());
};
void MainCamera::OnDestroy() {
	CameraSystem::OnExit();
};

// Camera component data in editor
void MainCamera::DrawInInspector() {};

// Save / load camera component from file
void MainCamera::Serialize(Json::Value& /*outComp*/) const {};
void MainCamera::Deserialize(const Json::Value& /*compObj*/) {};

void MainCamera::CopyFrom(Component* /*src*/) {};

std::unique_ptr<Component> MainCamera::Clone(GameObject& go)
{
	auto n = std::make_unique<MainCamera>(go);
	n.get()->CopyFrom(this);
	return n;
}