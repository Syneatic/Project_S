#include "camera.hpp"
#include "gameobject.hpp"
#include "controller_components.hpp"
#include "scene.hpp"
#include "physics.hpp"
#include "physics_types.hpp"


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
    spawnPoint = _transform.position, moveStartPos = _transform.position;;

    rb = _owner.GetComponent<RigidBody>();
    noiseSource = _owner.GetComponent<NoiseSource>();
    rockObject = SceneManager::ActiveScene()->FindGameObjectByName("Rock");
    spawnPoint = _transform.position;

    _owner.Subscribe<OnCollisionEvent>(
        &OnCollisionEvent::self,
        &_owner,
        [this](const OnCollisionEvent& e)
        {
            this->HandleCollision(e);
        }
    );

    _owner.Subscribe<OnTriggerEvent>(
        &OnTriggerEvent::self,
        &_owner,
        [this](const OnTriggerEvent& e)
        {
            this->HandleTrigger(e);
        }
    );
}

void PlayerController::OnUpdate()
{
    if (!rb) return;

	//we use a ray cast downwards to check if the player is grounded
    RaycastHit hit;
    //or use whatever range u want
    _isGrounded = Physics::Raycast(_transform.position, float2(0.f, -1.f), _transform.scale.y / 2.f + 10.f, hit, 1 << 1);

    float input = 0.f;


    //===================|Movement Speed|===================
    if (AEInputCheckCurr(AEVK_A))   input -= 1.f;
    if (AEInputCheckCurr(AEVK_D))   input += 1.f;

    float acceleration = maxSpeed / time;

    if (input != 0.f)   
        rb->velocity.x += input * acceleration * EngineCTX::dt;
    //else 
    //{
    //    float friction = acceleration;
    //    //friction
    //    if (rb->velocity.x > 0.f)
    //        rb->velocity.x = std::max(0.f, rb->velocity.x - friction * static_cast<f32>(EngineCTX::dt));
    //    else if (rb->velocity.x < 0.f) {
    //        rb->velocity.x = std::min(0.f, rb->velocity.x + friction * static_cast<f32>(EngineCTX::dt));
    //    }
    //}
    rb->velocity.x = std::clamp(rb->velocity.x, -maxSpeed, maxSpeed);

    //===================|Jump Mechanic|=====================

    float timerToReach = 2.f;

    //Once if space is pressed once
    if (AEInputCheckTriggered(AEVK_SPACE) && _isGrounded)
    {           
        //Set the space bar velocity to true
        //Check if the player reach the height (dt)
        float jumpSpeed = std::sqrt(timerToReach * Physics::gravity * jumpHeight);
        rb->AddForce({0.f,jumpSpeed * 75.f});
    }

    //===================|Throw Mechanic|=====================
    if (AEInputCheckTriggered(AEVK_R))
    {
        if (!rockObject) return;

        if (!rockObject->active())
        {
            if (auto* rc = rockObject->GetComponent<RockController>())
            {
                rockObject->active(true);
                rc->Throw(_transform.position);
            }
        }

        //Ignore's the projectile LayerMask
        auto* col = _owner.GetComponent<BoxCollider>();
        if (col)
        {
            ignoreProjectileCollision = true;
            ignoreTimer = 0.f;

            col->collisionMask &= ~(1 << 3);
        }
    }

    //Turn projectile LayerMask back on after a timer
    if (ignoreProjectileCollision)
    {
        ignoreTimer += EngineCTX::dt;

        if (ignoreTimer >= ignoreDuration)
        {
            ignoreProjectileCollision = false;

            auto* col = _owner.GetComponent<BoxCollider>();
            if (col)
            {
                col->collisionMask |= (1 << 3);
            }
        }
    }

    //===================|Echo Mechanic|=====================

    if (AEInputCheckTriggered(AEVK_E)/* && echoTimer >= echoCooldown*/)
    {
        if (noiseSource)
            noiseSource->Emit(_transform.position);
    }

    float2 currentPos = _owner.worldTransform().position;
    f32 speed = length(rb->velocity);

    if (speed >= echoDistanceThreshold)
    {
        distanceAccumulated += abs(currentPos.x - lastEchoPos.x);

        if (distanceAccumulated >= echoDistanceThreshold)
        {
            distanceAccumulated = 0.f;

            if (noiseSource) noiseSource->Emit(currentPos);
        }
    }

    lastEchoPos = currentPos;
}

void PlayerController::OnDestroy()
{

}

void PlayerController::Respawn()
{
    _transform.position = spawnPoint;
    //rb->HitEnemy = false;
}

void PlayerController::SaveSpawn(const float2& pos)
{
    spawnPoint = pos;
    Debug::Log("Spawn saved at: %f, %f", pos.x, pos.y);
}

void PlayerController::HandleCollision(const OnCollisionEvent& e)
{
    auto* col = e.other->GetComponent<BoxCollider>();

    if (col->layer == 1 << 2)
    {
        //==================|Collision with Enemy|=======================
        Debug::Log("Player hit enemy -> Respawn");
        Respawn();
    }

    if (col->layer == 1 << 3)
    {
        Debug::Log("Rock picked up");

        e.other->active(false);

        if (auto* rc = e.other->GetComponent<RockController>())
            rc->ResetRock();
    }
}

void PlayerController::HandleTrigger(const OnTriggerEvent& e)
{
    auto* col = e.other->GetComponent<BoxCollider>();
    if (col->layer == 1 << 4)
    {
        Debug::Log("Checkpoint Reached");
        SaveSpawn(e.other->transform().position);
    }
}

void PlayerController::CopyFrom(Component* src)
{
    auto s = dynamic_cast<PlayerController*>(src);
    if (!s) return;

    maxSpeed = s->maxSpeed;
    jumpHeight = s->jumpHeight;
    time = s->time;

    spawnPoint = s->spawnPoint;
}

std::unique_ptr<Component> PlayerController::Clone(GameObject& go)
{
    auto n = std::make_unique<PlayerController>(go);
    n.get()->CopyFrom(this);
    return n;
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

    _transform.position = playerPos + dir * 20.f;
    rb->velocity = dir * throwSpeed;
    rb->useGravity = true;
}

void RockController::OnImpact(const OnCollisionEvent& e)
{
    float2 vel = normalize(rb->velocity) + (-e.normal);
    rb->velocity = vel * 0.6f;
}

void RockController::ResetRock()
{

}

void RockController::CopyFrom(Component* src)
{
    auto s = dynamic_cast<RockController*>(src);
    if (!s) return;

    throwSpeed = s->throwSpeed;
    throwAngle = s->throwAngle;
}

std::unique_ptr<Component> RockController::Clone(GameObject& go)
{
    auto n = std::make_unique<RockController>(go);
    n.get()->CopyFrom(this);
    return n;
}



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
        ImGui::DragFloat("Time Interval", &groundEmitInterval, 1.5f);
        break;

    case EnemyType::Patrol:
        ImGui::DragFloat("Move Speed", &moveSpeed, 1.f);
        ImGui::DragFloat("Patrol Range", &patrolRange, 10.f);
        ImGui::DragFloat("Time Interval", &groundEmitInterval, 1.5f);
        break;

    case EnemyType::Flying:
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
        outComp["groundEmitInterval"] = groundEmitInterval;
        break;

    case EnemyType::Patrol:
        outComp["moveSpeed"] = moveSpeed;
        outComp["groundEmitInterval"] = groundEmitInterval;
        break;

    case EnemyType::Flying:
        break;
    }
}

void EnemyController::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("enemyType") && compObj["enemyType"].isInt())
        type = static_cast<EnemyType>(compObj["enemyType"].asInt());

    if (compObj.isMember("groundEmitInterval")) groundEmitInterval = compObj["groundEmitInterval"].asFloat();

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
        break;
    }
}

void EnemyController::OnStart()
{
    rb = _owner.GetComponent<RigidBody>();
    ns = _owner.GetComponent<NoiseSource>();

    rb = _owner.GetComponent<RigidBody>();
    playerObject = SceneManager::ActiveScene()->FindGameObjectByName("Player");

    if (type == EnemyType::Drop)
    {
        if (rb)
        {
            rb->useGravity = false;
            rb->velocity = float2::zero();
        }
    }
    if (type == EnemyType::Patrol)
    {
        rb->useGravity = true;
        rb->velocity = float2::zero();

        //if (rb->isGrounded)
            //rb->useGravity = false;
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

    RaycastHit hit;
    bool hasLanded = false;
    f32 xDist = absf(playerObject->transform().position.x - _transform.position.x);
    f32 yDist = _transform.position.y - playerObject->transform().position.y;

    if (xDist <= 50.f && yDist > 0 && yDist <= 100.f) {
        rb->useGravity = true;
        hasDropped = true;
    }
    float2 origin = _transform.position;
    float2 dir = float2(0, -1);

    uint32_t groundLayer = static_cast<uint32_t>(Layer::Environment);

    //if (hasDropped && rb->isGrounded) hasLanded = true;

    if (Physics::Raycast(origin, dir, 500.f, hit, groundLayer) && !hasLanded) {
        if (groundEmitTimer >= groundEmitInterval) {
            groundEmitTimer = 0.f;

            float2 emitPos = hit.point + hit.normal * 15.f;

            if (ns) ns->Emit(emitPos);
        }
    }
}

void EnemyController::UpdatePatrol() {
    if (!rb) return;

    //Raycast
    RaycastHit hit;

    //Ground Raycast Variables
    float halfWidth = _transform.scale.x / 2.f;
    float2 origin = _transform.position + float2(patrolDir * halfWidth, 0);
    float2 dir = float2(0, -1);

    //Side Raycast Variables
    float2 sideOrigin = _transform.position;
    float2 sideDir = float2((f32)patrolDir, 0);

    uint32_t wallLayer = static_cast<uint32_t>(Layer::Environment);

    //Once RayCast hits a wall it will flip velocity
    if (Physics::Raycast(sideOrigin, sideDir, halfWidth, hit, wallLayer))
    {
        patrolDir *= -1;
        return;
    }

    uint32_t groundLayer = static_cast<uint32_t>(Layer::Environment);

    bool wasNearEdge = false;
    bool groundAhead = Physics::Raycast(origin, dir, 40.f, hit, groundLayer);

    if (!groundAhead)
    {
        if (!wasNearEdge && groundEmitTimer >= groundEmitInterval)
        {
            groundEmitTimer = 0.f;
            if (ns) ns->Emit(_transform.position);
        }

        wasNearEdge = true;
        patrolDir *= -1;
        return;
    }
    else
    {
        wasNearEdge = false;
    }

    //f32 distance = _transform.position.x - startPos.x;

    //if (distance > patrolRange) patrolDir = -1;
    //if (distance < -patrolRange) patrolDir = 1;

    rb->velocity.x = patrolDir * moveSpeed;
}

void EnemyController::CopyFrom(Component* src)
{
    auto s = dynamic_cast<EnemyController*>(src);
    if (!s) return;

    //Global Variable
    groundEmitTimer = s->groundEmitTimer;
    groundEmitInterval = s->groundEmitInterval;

    //Drop Variable
    detectDistance = s->detectDistance;
    hasDropped = s->hasDropped;

    //Patrol Variable
    moveSpeed = s->moveSpeed;
    patrolRange = s->patrolRange;
    startPos = s->startPos;
    patrolDir = s->patrolDir; // 1 = right, -1 = left

    type = s->type;
}

std::unique_ptr<Component> EnemyController::Clone(GameObject& go)
{
    auto n = std::make_unique<EnemyController>(go);
    n.get()->CopyFrom(this);
    return n;
}
