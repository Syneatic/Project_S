#pragma once
#include <vector>
#include <string>

//forward decl
struct GameObject;

struct Scene
{
protected:
	std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	std::string _name{};

	void InitializeGameObjects();

public:
	virtual void OnEnter();
	virtual void OnUpdate();
	virtual void OnExit();

	//===== SERIALIZATION =====
	const std::string& name() const;
	const std::string& name(std::string name);
	std::vector<std::unique_ptr<GameObject>>& gameObjectList();
	const std::vector<std::unique_ptr<GameObject>>& gameObjectList() const;


	Scene();
	Scene(std::string name);
	virtual ~Scene();
};