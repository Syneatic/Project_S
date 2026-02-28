#include "camera.hpp"
#include "gameobject.hpp"
#include "controller_components.hpp"
#include "scene.hpp"


//===================|Player Controller|===================
void PlayerController::DrawInInspector()
{
    ImGui::TextUnformatted("Max Speed");
    ImGui::DragFloat("##max_speed", &maxSpeed, 0.1f);

    //ImGui::TextUnformatted("Acceleration");
    //ImGui::DragFloat("##Acceleration", &acceleration, 0.1f);

    ImGui::TextUnformatted("TimeToReach");
    ImGui::DragFloat("##TimeToReach", &time, 0.1f);

    ImGui::TextUnformatted("Jump Height");
    ImGui::DragFloat("##jumpHeight", &jumpHeight, 0.1f);
}

void PlayerController::Serialize(Json::Value& outComp) const
{
    outComp["max_speed"] = maxSpeed;
    outComp["time"] = time;
    outComp["jumpHeight"] = jumpHeight;
}

void PlayerController::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("max_speed") && compObj["max_speed"].isNumeric())
        maxSpeed = compObj["max_speed"].asFloat();

    if (compObj.isMember("jumpHeight") && compObj["jumpHeight"].isNumeric())
        jumpHeight = compObj["jumpHeight"].asFloat();

    if (compObj.isMember("time") && compObj["time"].isNumeric())
        time = compObj["time"].asFloat();
}

void PlayerController::OnStart()
{
    rb = _owner.GetComponent<RigidBody>();
    rockObject = SceneManager::ActiveScene()->FindGameObjectByName("Rock");
}

void PlayerController::OnUpdate()
{
    dt = static_cast<f32>(AEFrameRateControllerGetFrameTime());
    if (!rb) return;

    float input = 0.f;


    //===================|Movement Speed|===================
    if (AEInputCheckCurr(AEVK_A))   input -= 1.f;
    if (AEInputCheckCurr(AEVK_D))   input += 1.f;

    float acceleration = maxSpeed / time;

    if (input != 0.f)   rb->velocity.x += input * acceleration * dt;
    else {
        float friction = acceleration;
        //friction
        if (rb->velocity.x > 0.f)
            rb->velocity.x = (std::max)(0.f, rb->velocity.x - friction * dt);
        else if (rb->velocity.x < 0.f) {
            rb->velocity.x = (std::min)(0.f, rb->velocity.x + friction * dt);
        }
    }
    rb->velocity.x = std::clamp(rb->velocity.x, -maxSpeed, maxSpeed);

    //===================|Jump Mechanic|=====================

    float timerToReach = 2.f;

    //Once if space is pressed once
    if (AEInputCheckTriggered(AEVK_SPACE) && rb->Is_Grounded)
    {           
        //Set the space bar velocity to true
        //Check if the player reach the height (dt)
        float jumpSpeed = std::sqrt(timerToReach * rb->gravity * jumpHeight);
        rb->velocity.y = jumpSpeed;
        rb->Is_Grounded = false;
    }

    //===================|Throw Mechanic|=====================
    if (AEInputCheckTriggered(AEVK_R))
    {
        if (rockObject)
        {
            auto* rc = rockObject->GetComponent<RockController>();

            if (rc)
                rc->Throw(_transform.position);
        }
    }
}

void PlayerController::OnDestroy()
{

}



//===================|Rock Controller|===================
void RockController::DrawInInspector()
{
    ImGui::TextUnformatted("Throw Speed");
    ImGui::DragFloat("##throwSpeed", &throwSpeed, 0.1f);

    ImGui::TextUnformatted("Throw Angle");
    ImGui::DragFloat("##throwAngle", &throwAngle, 0.f, 360.f);

}

void RockController::Serialize(Json::Value& outComp) const
{
    outComp["throwSpeed"] = throwSpeed;
    outComp["throwAngle"] = throwAngle;
}

void RockController::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("throwSpeed") && compObj["throwSpeed"].isNumeric())
        throwSpeed = compObj["throwSpeed"].asFloat();

    if (compObj.isMember("throwAngle") && compObj["throwAngle"].isNumeric())
        throwAngle = compObj["throwAngle"].asFloat();
}

void RockController::OnStart()
{
    rb = _owner.GetComponent<RigidBody>();

    EventHandler::SubscribeFilter<OnCollisionEvent, GameObject*>(
        &OnCollisionEvent::self,
        &_owner,
        [this](const OnCollisionEvent& e)
        {
            this->OnImpact(e);
        }
    );
}

void RockController::OnUpdate()
{
    if (!rb) return;
}

void RockController::OnDestroy()
{

}


//=========|Rock Mechanic Helper Function|==================
void RockController::Throw(const float2& playerPos)
{
    if (!rb) return;

    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    float2 mouseWorld = CameraSystem::ScreenToWorld(float2(static_cast<f32>(mouseX), static_cast<f32>(mouseY)));

    float2 dir = mouseWorld - playerPos;

    if (length(dir) < 0.001f)
        return;

    dir = normalize(dir);

    _transform.position = playerPos + dir * 10.f;
    rb->velocity = dir * throwSpeed;
    rb->Affected_By_Gravity = true;

    rb->HitEnvironment = false;
    rb->Is_Grounded = false;
}

void RockController::OnImpact(const OnCollisionEvent& e)
{
    float2 vel = normalize(rb->velocity) + (-e.normal);
    rb->velocity = vel * e.impulse * 0.6f;
}

void RockController::ResetRock()
{

}

