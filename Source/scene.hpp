/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

//forward decl
class GameObject;

class Scene
{
protected:
	std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	std::string _name{};

	void InitializeGameObjects();

public:
	virtual void OnEnter();
	virtual void OnUpdate();
	virtual void OnExit();
	virtual bool IsEditorScene() const { return false; }

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