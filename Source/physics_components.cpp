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
    case Layer::End: return "End";
    default: return "Untagged";
    }
}

// ===== COLLIDER DEFINITIONS =====
void Collider::OnStart()
{
    Physics::RegisterCollider(this);
}

void Collider::DrawBaseInspector()
{
    ImGui::SeparatorText("Collision Layer");
    const std::array<const char*, 7> layerNames[] =
    {
        "Nothing",
        "Player",
        "Environment",
        "Enemy",
        "Projectile",
        "CheckPoint",
        "End"
    };

    int currentLayer = 0;
    for (int i = 1; i < (int)layerNames->size(); i++)
    {
        if (layer == (1u << (i - 1)))
        {
            currentLayer = i;
            break;
        }
    }

    ImGui::TextUnformatted("Layer");
    if (ImGui::Combo("##Layer", &currentLayer, layerNames->data(), layerNames->size()))
    {
        layer = static_cast<u32>(currentLayer == 0 ? 0 : 1 << (currentLayer - 1));
    }
    ImGui::Separator();


    ImGui::TextUnformatted("Collision Mask");
    if (ImGui::BeginCombo("##collisionmask", "Collision Mask"))
    {
        for (int i = 0; i < layerNames->size(); i++)
        {
            u32 bit = (i == 0) ? 0u : (1u << (i - 1));
            bool selected = (i == 0) ? (collisionMask == 0) : (collisionMask & bit) != 0;

            if (ImGui::Selectable(layerNames->data()[i], selected,ImGuiSelectableFlags_NoAutoClosePopups))
            {
                if (i == 0)
                    collisionMask = 0;
                else
                    collisionMask ^= bit;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SeparatorText("Properties");
    ImGui::Checkbox("IsTrigger##collider", &isTrigger);
}

// ===== COLLIDER DEFINITIONS =====





// ===== BOX COLLIDER DEFINITIONS =====
void BoxCollider::DrawInInspector()
{
    DrawBaseInspector();
    ImGui::TextUnformatted("Size");
    ImGui::DragFloat2("##boxcollider_size", &size.x, 0.1f);
}

void BoxCollider::Serialize(Json::Value& outComp) const
{
    outComp["isTrigger"] = isTrigger;
    outComp["size"] = WriteFloat2(size);
    outComp["layerMask"] = layer;
    outComp["collisionMask"] = collisionMask;
}

void BoxCollider::Deserialize(const Json::Value& compObj)
{
    isTrigger = compObj["isTrigger"].asBool();
    if (compObj.isMember("size")) ReadFloat2(compObj["size"], size);
    if (compObj.isMember("layerMask")) layer = compObj["layerMask"].asUInt();
    if (compObj.isMember("collisionMask")) collisionMask = compObj["collisionMask"].asUInt();
}

void BoxCollider::CopyFrom(Component* src)
{
    auto s = dynamic_cast<BoxCollider*>(src);
    if (!s) return;
    isTrigger = s->isTrigger;
    layer = s->layer;
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
	ImGui::Checkbox("IsStatic", &isStatic);
    ImGui::Checkbox("isKinematic", &isKinematic);
	ImGui::Checkbox("Affected By Gravity", &useGravity);
	//ImGui::DragFloat2("Velocity", &velocity.x, 0.1f);
} 

void RigidBody::Serialize(Json::Value& outComp) const
{
	outComp["Is Static"] = isStatic;
	outComp["isKinematic"] = isKinematic;
	outComp["Affected By Gravity"] = useGravity;
}

void RigidBody::Deserialize(const Json::Value& compObj)
{
	if (compObj.isMember("Is Static") && compObj["Is Static"].isBool())
		isStatic = compObj["Is Static"].asBool();

    if (compObj.isMember("isKinematic") && compObj["isKinematic"].isBool())
        isKinematic = compObj["isKinematic"].asBool();

	if (compObj.isMember("Affected By Gravity") && compObj["Affected By Gravity"].isBool())
		useGravity = compObj["Affected By Gravity"].asBool();
}

void RigidBody::OnStart()
{
    Physics::RegisterRigidBody(this);
}

void RigidBody::CopyFrom(Component* src)
{
    auto s = dynamic_cast<RigidBody*>(src);
    if (!s) return;
    useGravity = s->useGravity;
    isStatic = s->isStatic;
	isKinematic = s->isKinematic;
}

std::unique_ptr<Component> RigidBody::Clone(GameObject& go)
{
    auto n = std::make_unique<RigidBody>(go);
    n.get()->CopyFrom(this);
    return n;
}

// ===== RIGIDBODY DEFINITIONS =====
