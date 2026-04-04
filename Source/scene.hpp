/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

//forward decl
class GameObject;

//represents a scene in the game, contains gameobjects and handles their lifecycle
class Scene
{
protected:
	std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	std::string _name{};

	void InitializeGameObjects();

public:
	//lifecycle functions
	virtual void OnEnter();
	virtual void OnUpdate();
	virtual void OnExit();

	//===== SERIALIZATION =====
	std::string& name();
	const std::string& cname() const;
	const std::string& name(std::string name);
	std::vector<std::unique_ptr<GameObject>>& gameObjectList();
	const std::vector<std::unique_ptr<GameObject>>& gameObjectList() const;
	GameObject* FindGameObjectByName(const std::string& name);

	Scene();
	Scene(std::string name);
	virtual ~Scene();
};