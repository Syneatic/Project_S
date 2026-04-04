/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

//fwd decl
class GameObject;
struct Transform;

//abstract base class for all components to inherit
class Component
{
protected:
    GameObject& _owner;
    Transform& _transform;
    Transform& _worldTransform;
    bool _active{ true };

public:
	// ===== ACCESSORS =====
    GameObject& gameObject() { return _owner; }
    const GameObject& gameObject() const { return _owner; }
    friend class GameObject; //allow GameObject class to access private and protected

    Transform& transform() { return _transform; }
    const Transform& transform() const { return _transform; }
    Transform& worldTransform() { return _worldTransform; }
    const Transform& worldTransform() const { return _worldTransform; }

    bool& active() { return _active; }
	bool active() const { return _active; }
    bool active(bool state) { return _active = state; }

	// ===== INSPECTOR & SERIALIZATION =====    
	//draw component fields in inspector
	virtual void DrawInInspector() {}; 
	//serialize component fields to json
	virtual void Serialize(Json::Value& /*outComp*/) const {}; 
	//deserialize component fields from json
	virtual void Deserialize(const Json::Value& /*compObj*/) {}; 

	// ===== LIFECYCLE =====
	//called once when the scene starts
	virtual void OnStart() {}; 
	//called every frame if component is active
	virtual void OnUpdate() {}; 
	//called once when the scene ends or component is removed
	virtual void OnDestroy() {}; 

	//returns the name of the component type, used for inspector and serialization
	virtual const std::string name() const = 0; 
	//virtual destructor since we will be deleting derived components through base pointers
	virtual ~Component() = default; 
	//constructor that initializes reference to owner GameObject and its transforms
	Component(GameObject& owner);

	//copies all fields from the given source component, used for cloning and copying components in the inspector
	virtual void CopyFrom(Component* src) = 0; 
	//returns a deep copy of this component attached to the given GameObject, used for cloning and copying components in the inspector
	virtual std::unique_ptr<Component> Clone(GameObject& go) = 0; 

};