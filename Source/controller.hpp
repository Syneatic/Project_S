#include "ImGUI/imgui.h"
#include "AEEngine.h"

#include <string>

#include "component.hpp"
#include "gameobject.hpp"
#include "math.hpp"

//abstract
struct Controller : Behaviour
{
    //max speed
    //velocity
    //drag
    //gravity
    //jump height
    //collidertype
};

struct PlayerController : Controller
{
    f32 maxspeed{ 200.f };
    f32 jumpHeight{ 20.f };
    float2 velocity{ 1.f, 1.f };
    f32 gravity{ 12.f };
    f32 drag{ 10.f };

    void DrawInInspector() override
    {
        f32 total_width = ImGui::GetContentRegionAvail().x;
        f32 spacing = ImGui::GetStyle().ItemSpacing.x;

        ImGui::TextUnformatted("Max Speed");
        ImGui::DragFloat("##max_speed", &maxspeed, 0.1f, 0.f);

        ImGui::TextUnformatted("Jump Height");
        ImGui::DragFloat("##jumpHeight", &jumpHeight, 0.1f, 0.f);

        ImGui::TextUnformatted("Velocity");
        ImGui::PushItemWidth(((total_width - spacing) / 2) - 100.f);
        ImGui::DragFloat("##velocityX", &velocity.x, 0.f, -maxspeed, maxspeed);
        ImGui::SameLine();
        ImGui::DragFloat("##velocityY", &velocity.y, 0.f, -100.f, jumpHeight);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Velocity"))
        {
            velocity.x = 0.0f;
            velocity.y = 0.0f;
        }
        ImGui::PopItemWidth();

        ImGui::TextUnformatted("Gravity");
        ImGui::DragFloat("##gravity", &gravity, 0.1f, 0.f);

        ImGui::TextUnformatted("Drag");
        ImGui::DragFloat("##drag", &drag, 0.1f, 0.f);
    }

    void Serialize(Json::Value& outComp) const override
    {
        outComp["maxSpeed"] = maxspeed;
        outComp["velocty"] = WriteFloat2(velocity);
        outComp["gravity"] = gravity;
        outComp["jumpHeight"] = jumpHeight;
        outComp["drag"] = drag;
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("maxSpeed") && compObj["maxSpeed"].isNumeric())
            maxspeed = compObj["maxSpeed"].asFloat();
        if (compObj.isMember("velocity"))
            ReadFloat2(compObj["velocity"], velocity);
        if (compObj.isMember("gravity") && compObj["gravity"].isNumeric())
            gravity = compObj["gravity"].asFloat();
        if (compObj.isMember("jumpHeight") && compObj["jumpheight"].isNumeric())
            jumpHeight = compObj["jumpHeight"].asFloat();
        if (compObj.isMember("drag") && compObj["drag"].isNumeric())
            drag = compObj["drag"].asFloat();
    }

    void OnStart() override
    {
    } 

    void OnUpdate() override
    {
        GameObject& owner = *_owner;
        Transform& trans = *owner.GetComponent<Transform>();

        if (AEInputCheckTriggered(AEVK_A))
        {
            trans.position.x -= 2.0f;
        }
    }

    void OnDestroy() override
    {

    }

    const std::string name() const override { return "PlayerController"; }
};