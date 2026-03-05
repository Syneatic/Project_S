#include "physics.hpp"
#include "physics_components.hpp"
#include "base_components.hpp"

const char* LayerToString(Layer layer)
{
    switch (layer)
    {
    case Layer::Nothing: return "Nothing";
    case Layer::Player: return "Player";
    case Layer::Enemy: return "Enemy";
    case Layer::Environment: return "Environment";
    case Layer::Projectile: return "Projectile";
    case Layer::CheckPoint: return "CheckPoint";
    default: return "Untagged";
    }
}

// ===== COLLIDER DEFINITIONS =====
void Collider::OnStart()
{
    Physics::RegisterCollider(this);
}

bool Collider::Has_Layer(Layer layer) const
{
    return (layerMask & static_cast<uint32_t>(layer)) != 0;
}

void Collider::Add_Layer(Layer layer)
{
    layerMask |= static_cast<uint32_t>(layer);
}

void Collider::Remove_Layer(Layer layer)
{
    layerMask &= ~static_cast<uint32_t>(layer);
}

void Collider::Add_CollisionLayer(Layer layer)
{
    collisionMask |= static_cast<uint32_t>(layer);
}

void Collider::Remove_CollisionLayer(Layer layer)
{
    collisionMask &= ~static_cast<uint32_t>(layer);
}

bool Collider::CollidesWithLayer(Layer layer)const
{
    return (collisionMask & static_cast<uint32_t>(layer));
}

bool Collider::ShouldCollide(const Collider& other)const
{
    return (collisionMask & other.layerMask);
}

void Collider::DrawLayerInInspector()
{
    ImGui::Separator();
    ImGui::Text("I am (Layer):");

    const char* layerNames[] = {
    "Nothing", "Player", "Environment", "Enemy", "Projectile", "CheckPoint"
    };

    Layer layers[] = {
    Layer::Nothing, Layer::Player, Layer::Environment,
    Layer::Enemy, Layer::Projectile, Layer::CheckPoint
    };

    int currentLayer = 0;
    for (int i = 0; i < 6; i++)
    {
        if (Has_Layer(layers[i]))
        {
            currentLayer = i;
            break;
        }
    }

    if (ImGui::Combo("##Layer", &currentLayer, layerNames, 6))
    {
        layerMask = static_cast<uint32_t>(layers[currentLayer]);
    }


    ImGui::Separator();

    ImGui::Text("I collide with (Mask):");

    bool collidesPlayer = CollidesWithLayer(Layer::Player);
    bool collidesEnemy = CollidesWithLayer(Layer::Enemy);
    bool collidesEnvironment = CollidesWithLayer(Layer::Environment);
    bool collidesProjectile = CollidesWithLayer(Layer::Projectile);
    bool collidesCheckPoint = CollidesWithLayer(Layer::CheckPoint);


    if (ImGui::Checkbox("PlayerMask", &collidesPlayer))
    {
        if (collidesPlayer) Add_CollisionLayer(Layer::Player);
        else Remove_CollisionLayer(Layer::Player);
    }

    if (ImGui::Checkbox("EnemyMask", &collidesEnemy))
    {
        if (collidesEnemy) Add_CollisionLayer(Layer::Enemy);
        else Remove_CollisionLayer(Layer::Enemy);
    }

    if (ImGui::Checkbox("EnvironmentMask", &collidesEnvironment))
    {
        if (collidesEnvironment) Add_CollisionLayer(Layer::Environment);
        else Remove_CollisionLayer(Layer::Environment);
    }

    if (ImGui::Checkbox("ProjectileMask", &collidesProjectile))
    {
        if (collidesProjectile) Add_CollisionLayer(Layer::Projectile);
        else Remove_CollisionLayer(Layer::Projectile);
    }

    if (ImGui::Checkbox("CheckPointMask", &collidesCheckPoint))
    {
        if (collidesProjectile) Add_CollisionLayer(Layer::CheckPoint);
        else Remove_CollisionLayer(Layer::CheckPoint);
    }
}

// ===== COLLIDER DEFINITIONS =====





// ===== BOX COLLIDER DEFINITIONS =====
void BoxCollider::DrawInInspector()
{
    DrawLayerInInspector();
    ImGui::Separator();
    ImGui::TextUnformatted("Size");
    ImGui::DragFloat2("##boxcollider_size", &size.x, 0.1f);
}

void BoxCollider::Serialize(Json::Value& outComp) const
{
    outComp["size"] = WriteFloat2(size);
    outComp["layerMask"] = layerMask;
    outComp["collisionMask"] = collisionMask;
}

void BoxCollider::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("size")) ReadFloat2(compObj["size"], size);
    if (compObj.isMember("layerMask")) layerMask = compObj["layerMask"].asUInt();
    if (compObj.isMember("collisionMask")) collisionMask = compObj["collisionMask"].asUInt();
}

void BoxCollider::CopyFrom(Component* src)
{
    auto s = dynamic_cast<BoxCollider*>(src);
    if (!s) return;
    layerMask = s->layerMask;
    collisionMask = s->collisionMask;
    size = s->size;
}

std::unique_ptr<Component> BoxCollider::Clone(GameObject& go)
{
    auto n = std::make_unique<BoxCollider>(go);
    n.get()->CopyFrom(this);
    return n;
}

// ===== BOX COLLIDER DEFINITIONS =====




// ===== RIGIDBODY DEFINITIONS =====
void RigidBody::DrawInInspector()
{ 
	ImGui::Checkbox("Is Static", &isStatic);
	ImGui::Checkbox("Affected By Gravity", &affectedByGravity);
	ImGui::Checkbox("Grounded", &isGrounded);
	ImGui::DragFloat("Gravity", &gravity, 0.1f);
	ImGui::DragFloat2("Velocity", &velocity.x, 0.1f);
} 

void RigidBody::Serialize(Json::Value& outComp) const
{
	outComp["Is Static"] = isStatic;
	outComp["Affected By Gravity"] = affectedByGravity;
	outComp["Grounded"] = isGrounded;
	outComp["Gravity"] = gravity;
	outComp["Velocity"] = velocity.x;
}

void RigidBody::Deserialize(const Json::Value& compObj)
{
	if (compObj.isMember("Is Static") && compObj["Is Static"].isBool())
		isStatic = compObj["Is Static"].asBool();
	if (compObj.isMember("Affected By Gravity") && compObj["Affected By Gravity"].isBool())
		affectedByGravity = compObj["Affected By Gravity"].asBool();
	if (compObj.isMember("Grounded") && compObj["Grounded"].isBool())
		isGrounded = compObj["Grounded"].asBool();
	if (compObj.isMember("Gravity") && compObj["Gravity"].isNumeric())
		gravity = compObj["Gravity"].asFloat();
	if (compObj.isMember("Velocity") && compObj["Velocity"].isNumeric())
		velocity.x = compObj["Velocity"].asFloat();
}

void RigidBody::OnStart()
{
    Physics::RegisterRigidBody(this);
}

void RigidBody::Clear_Forces()
{ 
	if (isStatic) 
	{ 
		velocity = float2::zero(); 
	} 
} 

void RigidBody::CopyFrom(Component* src)
{
    auto s = dynamic_cast<RigidBody*>(src);
    if (!s) return;
    affectedByGravity = s->affectedByGravity;
    isStatic = s->isStatic;
    isGrounded = s->isGrounded;
    HitEnvironment = s->HitEnvironment;
    HitEnemy = s->HitEnemy;
    HitCheckPoint = s->HitCheckPoint;
    HitProjectile = s->HitProjectile;
    gravity = s->gravity;
    terminalVelocity = s->terminalVelocity;
    velocity = s->velocity;
}

std::unique_ptr<Component> RigidBody::Clone(GameObject& go)
{
    auto n = std::make_unique<RigidBody>(go);
    n.get()->CopyFrom(this);
    return n;
}

// ===== RIGIDBODY DEFINITIONS =====
