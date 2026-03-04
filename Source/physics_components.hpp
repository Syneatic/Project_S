#pragma once 

struct Collision;

enum class Layer : uint32_t
{
    Nothing = 0,
    Player = 1 << 0,
    Environment = 1 << 1,
    Enemy = 1 << 2,
    Projectile = 1 << 3, //not for raycast for other object projectiles
    CheckPoint = 1 << 4 
};

const char* LayerToString(Layer layer);

//abstract
class Collider : public Component
{
public:
    uint32_t layerMask{ static_cast<uint32_t>(Layer::Nothing) };
    uint32_t collisionMask{ 0xFFFFFFFF };

    using CollisionCallback = std::function<void(const Collision&)>;
    std::vector<CollisionCallback> onCollisionListeners;

    void OnStart() override;
    void OnCollision(const Collision& data);

    bool Has_Layer(Layer layer) const;
    void Add_Layer(Layer layer);
    void Remove_Layer(Layer layer);
    void Add_CollisionLayer(Layer layer);
    void Remove_CollisionLayer(Layer layer);
    bool CollidesWithLayer(Layer layer)const;
    bool ShouldCollide(const Collider& other)const;
    void DrawLayerInInspector();

    Collider(GameObject& go) : Component(go) {};
};

class CircleCollider : public Collider
{
public:
    f32 radius{ 1.f };

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

    const std::string name() const override { return "CircleCollider"; }

    CircleCollider(GameObject& go) : Collider(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};

class BoxCollider : public Collider
{
public:
    float2 size{ 1.f,1.f };

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

    const std::string name() const override { return "BoxCollider"; }

    BoxCollider(GameObject& go) : Collider(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};

class RigidBody : public Component
{ 
public:
	bool Affected_By_Gravity{ false };
	bool Is_Static{ false };
	bool Is_Grounded{ false };
    bool HitEnvironment{ false };
    bool HitEnemy{ false };
    bool HitCheckPoint{ false };
    bool HitProjectile{ false };
	float gravity{ 9.8f };
	float terminalVelocity{ 12.0f };
	float2 velocity{ 0.0f,0.0f };

	void DrawInInspector()override;
	void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;


    void OnStart() override;
    void Clear_Forces();

	const std::string name() const override { return "RigidBody"; }

    RigidBody(GameObject& go) : Component(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};