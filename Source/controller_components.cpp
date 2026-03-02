#include "camera.hpp"
#include "gameobject.hpp"
#include "controller_components.hpp"
#include "scene.hpp"
#include "physics.hpp"


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
    spawnPoint = _transform.position;
}

void PlayerController::OnUpdate()
{
    if (!rb) return;

    float input = 0.f;


    //===================|Movement Speed|===================
    if (AEInputCheckCurr(AEVK_A))   input -= 1.f;
    if (AEInputCheckCurr(AEVK_D))   input += 1.f;

    float acceleration = maxSpeed / time;

    if (input != 0.f)   rb->velocity.x += input * acceleration * EngineCTX::dt;
    else {
        float friction = acceleration;
        //friction
        if (rb->velocity.x > 0.f)
            rb->velocity.x = std::max(0.f, rb->velocity.x - friction * static_cast<f32>(EngineCTX::dt));
        else if (rb->velocity.x < 0.f) {
            rb->velocity.x = std::min(0.f, rb->velocity.x + friction * static_cast<f32>(EngineCTX::dt));
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

    //==================|Collision with Enemy|=======================
    if (rb->HitEnemy) Respawn();
    if (rb->HitCheckPoint) {
        spawnPoint = _transform.position;
        rb->HitCheckPoint = false;
    }
}

void PlayerController::OnDestroy()
{

}

void PlayerController::Respawn()
{
    _transform.position = spawnPoint;
    rb->HitEnemy = false;
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


//enum class EnemyType {
//    Static = 0,
//    Drop,
//    Patrol,
//    Flying
//};

//===================|Enemy Controller|===================
void EnemyController::DrawInInspector()
{
    const char* types[] = { "Static", "Drop", "Patrol", "Flying" };

    int current = static_cast<int>(type);

    if (ImGui::Combo("Enemy Type", &current, types, IM_ARRAYSIZE(types)))
        type = static_cast<EnemyType>(current);

    ImGui::Separator();

    switch (type)
    {
    case EnemyType::Static:
        break;
    case EnemyType::Drop:
        ImGui::DragFloat("Detect Distance", &detectDistance, 10.f);
        break;

    case EnemyType::Patrol:
        ImGui::DragFloat("Move Speed", &moveSpeed, 1.f);
        ImGui::DragFloat("Patrol Range", &patrolRange, 10.f);
        break;

    case EnemyType::Flying:
        ImGui::DragFloat("Dive Speed", &diveSpeed, 1.f);
        ImGui::DragFloat("Detect Radius", &detectRadius, 10.f);
        break;
    }
}

void EnemyController::Serialize(Json::Value& outComp) const
{
    outComp["enemyType"] = static_cast<int>(type);

    switch (type)
    {
    case EnemyType::Drop:
        outComp["detectDistance"] = detectDistance;
        break;

    case EnemyType::Patrol:
        outComp["moveSpeed"] = moveSpeed;
        break;

    case EnemyType::Flying:
        outComp["diveSpeed"] = diveSpeed;
        outComp["detectRadius"] = detectRadius;
        break;
    }
}

void EnemyController::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("enemyType") && compObj["enemyType"].isInt())
        type = static_cast<EnemyType>(compObj["enemyType"].asInt());

    //Specific Enemy Types
    switch (type)
    {
    case EnemyType::Drop:
        if (compObj.isMember("detectDistance")) detectDistance = compObj["detectDistance"].asFloat();
        break;

    case EnemyType::Patrol:
        if (compObj.isMember("moveSpeed")) moveSpeed = compObj["moveSpeed"].asFloat();
        if (compObj.isMember("patrolRange")) patrolRange = compObj["patrolRange"].asFloat();
        break;

    case EnemyType::Flying:
        if (compObj.isMember("diveSpeed")) diveSpeed = compObj["diveSpeed"].asFloat();
        if (compObj.isMember("detectRadius")) detectRadius = compObj["detectRadius"].asFloat();
        break;
    }
}

void EnemyController::OnStart()
{
    rb = _owner.GetComponent<RigidBody>();
    ns = _owner.GetComponent<NoiseSource>();

    if (type == EnemyType::Drop)
    {
        if (rb)
        {
            rb->Affected_By_Gravity = false;
            rb->velocity = float2::zero();
        }
    }
    if (type == EnemyType::Patrol)
    {
        if (rb)
        {
            rb->Affected_By_Gravity = true;
        }
    }
}

void EnemyController::OnUpdate()
{
    groundEmitTimer += EngineCTX::dt;
    //Transform* trans = _owner.GetComponent<Transform>();
    if (!rb) return;

    switch (type)
    {
    case EnemyType::Drop:
        UpdateDrop();
        break;

    case EnemyType::Patrol:
        UpdatePatrol();
        break;

    default:
        break;
    }
}

void EnemyController::OnDestroy()
{

}


void EnemyController::UpdateDrop() {
    if (!rb) return;

    Physics::RaycastHit hit;

    float2 origin = _transform.position;
    float2 dir = float2(0, -1);

    uint32_t groundLayer = static_cast<uint32_t>(Layer::Environment);
    uint32_t playerLayer = static_cast<uint32_t>(Layer::Player);

    if (Physics::Raycast(origin, dir, 500.f, hit, groundLayer)){
        if (groundEmitTimer >= groundEmitInterval) {
            groundEmitTimer = 0.f;

            float2 emitPos = hit.point + hit.normal * 15.f;
            std::cout << "Drop ping" << std::endl;

            if (ns) ns->Emit(emitPos);
        }
    }

    if (Physics::Raycast(origin, dir, detectDistance, hit, playerLayer)) {
        rb->Affected_By_Gravity = true;
        hasDropped = true;
        std::cout << "player being collided" << std::endl;
    }
}

void EnemyController::UpdatePatrol() {
    if (!rb) return;

    //Raycast
    Physics::RaycastHit hit;

    //Ground Raycast Variables
    float2 origin = _transform.position + float2(patrolDir * 20.f, 0);
    float2 dir = float2(0, -1);

    //Side Raycast Variables
    float2 sideOrigin = _transform.position;
    float2 sideDir = float2((f32)patrolDir, 0);

    uint32_t wallLayer = static_cast<uint32_t>(Layer::Environment);

    //Once RayCast hits a wall it will flip velocity
    if (Physics::Raycast(sideOrigin, sideDir, _transform.scale.x / 2 + 10.f, hit, wallLayer))
    {
        patrolDir *= -1;
        return;
    }

    uint32_t groundLayer = static_cast<uint32_t>(Layer::Environment);

    if (!Physics::Raycast(origin, dir, _transform.scale.y / 2 + 10.f, hit, groundLayer)) {
        if (groundEmitTimer >= groundEmitInterval) {
            groundEmitTimer = 0.f;

            float2 emitPos{ _transform.position.x, _transform.position.y + _transform.scale.y / 2 };
            std::cout << "Drop ping" << std::endl;

            if (ns) ns->Emit(emitPos);
        }

        patrolDir *= -1;
        return; //exit so it doesn't trigger movement before rotating
    }

    f32 distance = _transform.position.x - startPos.x;

    if (distance > patrolRange) patrolDir = -1;
    if (distance < -patrolRange) patrolDir = 1;

    rb->velocity.x = patrolDir * moveSpeed;
}
