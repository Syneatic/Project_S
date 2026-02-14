#include <string>
#include <iostream>

#include "ImGUI/imgui.h"
#include "AEEngine.h"

#include "controller_components.hpp"

#include "math.hpp"
#include "gameobject.hpp"

#include "physics_components.hpp"


void PlayerController::DrawInInspector()
{
    ImGui::TextUnformatted("Max Speed");
    ImGui::DragFloat("##max_speed", &maxSpeed, 0.1f);

    ImGui::TextUnformatted("Jump Height");
    ImGui::DragFloat("##jumpHeight", &jumpHeight, 0.1f);
}

void PlayerController::Serialize(Json::Value& outComp) const
{
    outComp["max_speed"] = maxSpeed;
    outComp["jumpHeight"] = jumpHeight;
}

void PlayerController::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("max_speed") && compObj["max_speed"].isNumeric())
        maxSpeed = compObj["max_speed"].asFloat();
    if (compObj.isMember("jumpHeight") && compObj["jumpHeight"].isNumeric())
        jumpHeight = compObj["jumpHeight"].asFloat();
}

void PlayerController::OnStart()
{

} 

void PlayerController::OnUpdate()
{
    GameObject& owner = *_owner;
    Transform* trans = owner.GetComponent<Transform>();
    RigidBody* rb = owner.GetComponent<RigidBody>();
    
    if (!trans || !rb) return;

    rb->velocity.x = 0.f;

    if (AEInputCheckCurr(AEVK_A))
    {
        rb->velocity.x = -maxSpeed;
    }
    if (AEInputCheckCurr(AEVK_D))
    {
        rb->velocity.x = maxSpeed;
    }

    //Once if space is pressed once
    if (AEInputCheckTriggered(AEVK_SPACE) && rb->Is_Grounded)
    {           
        //Set the space bar velocity to true
        //Check if the player reach the height (dt)
        //rb.Affected_By_Gravity = false;
        rb->velocity.y = jumpHeight;
        rb->Is_Grounded = false;
        //t = 0.f;
    }
}

void PlayerController::OnDestroy()
{

}

