#pragma once
struct GameObject;

struct Component
{
protected:
    GameObject* _owner = nullptr;
    Component() = default;
    explicit Component(GameObject* owner) : _owner(owner) {};

public:

    void SetOwner(GameObject* owner) { _owner = owner; }
    GameObject& gameObject() { return *_owner; }
    const GameObject& gameObject() const { return *_owner; }
    friend struct GameObject; //allow GameObject class to access private and protected

	virtual void DrawInInspector() {};
    virtual void Serialize(Json::Value& /*outComp*/) const {};
    virtual void Deserialize(const Json::Value& /*compObj*/) {};

	virtual const std::string name() const = 0;
	virtual ~Component() = default;
};

struct Behaviour : Component
{
    virtual void OnStart() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnDestroy() = 0;

    virtual ~Behaviour() = default;
};