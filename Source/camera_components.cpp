#include "camera_components.hpp"
#include "gameobject.hpp"

void MainCamera::OnStart()
{
	CameraSystem::OnStart();
};
void MainCamera::OnUpdate() {
	CameraSystem::MoveCamera(_transform);
};
void MainCamera::OnDestroy() {
	CameraSystem::OnExit();
};

void MainCamera::DrawInInspector() {};
void MainCamera::Serialize(Json::Value& /*outComp*/) const {};
void MainCamera::Deserialize(const Json::Value& /*compObj*/) {};
void MainCamera::CopyFrom(Component* /*src*/) {};

std::unique_ptr<Component> MainCamera::Clone(GameObject& go)
{
	auto n = std::make_unique<MainCamera>(go);
	n.get()->CopyFrom(this);
	return n;
}