#pragma once

//fwd decl
class GameObject;
class Transform;

//ALL COMPONENTS ARE NOW BEHAVIOURS
class Component
{
protected:
    GameObject& _owner;
    Transform& _transform;

public:
    GameObject& gameObject() { return _owner; }
    const GameObject& gameObject() const { return _owner; }
    friend class GameObject; //allow GameObject class to access private and protected

    Transform& transform() { return _transform; }
    const Transform& transform() const { return _transform; }

	virtual void DrawInInspector() {};
    virtual void Serialize(Json::Value& /*outComp*/) const {};
    virtual void Deserialize(const Json::Value& /*compObj*/) {};

    virtual void OnStart() {};
    virtual void OnUpdate() {};
    virtual void OnDestroy() {};

	virtual const std::string name() const = 0;
	virtual ~Component() = default;
    Component(GameObject& owner);
};