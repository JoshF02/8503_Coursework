#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt)-> void
		{
			this->MoveLeft(dt);
		}
	);
	State* stateB = new State([&](float dt)-> void
		{
			this->MoveRight(dt);
		}
	);

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool
		{
			return this->counter > 3.0f;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool
		{
			return this->counter < 0.0f;
		}
	));
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -100, 0, 0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 100, 0, 0 });
	counter -= dt;
}
















EnemyGameObject::EnemyGameObject(PlayerGameObject* gameObject, GameWorld* world, float xMin, float xMax, float zMin, float zMax) {
    this->target = gameObject;
    this->stateMachine = new StateMachine();
    this->world = world;

    patrolPoints.push_back(Vector3(xMin, 2.5, zMin));
    patrolPoints.push_back(Vector3(xMax, 2.5, zMin));
    patrolPoints.push_back(Vector3(xMax, 2.5, zMax));
    patrolPoints.push_back(Vector3(xMin, 2.5, zMax));

    State* patrol = new State([&](float dt) -> void {
        Vector3 nextPatrolPointPos = patrolPoints[currentPatrolIndex];
        speed = 30.0f + speedBonus;

        if (currentPos.z > 1) { // if the enemy leaves its zone, next patrol point is the doorway back to its zone
            nextPatrolPointPos = Vector3(-160, 2.5, 1);
        }

        if ((nextPatrolPointPos - currentPos).LengthSquared() < 1.0f) {
            currentPatrolIndex++;
            currentPatrolIndex %= 4;
            nextPatrolPointPos = patrolPoints[currentPatrolIndex];
        }

        MoveToPosition(nextPatrolPointPos);
        });

    State* chase = new State([&](float dt) -> void {
        speed = 10.0f + speedBonus;
        MoveToPosition(Vector3(playerPos.x, 2.5, playerPos.z)); // keeps enemy on the ground
        });

    stateMachine->AddState(patrol);
    stateMachine->AddState(chase);

    stateMachine->AddTransition(new StateTransition(patrol, chase, [&]() -> bool {
        bool canTransition = (target->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Length() <= 45.0f;

        // if player within range then check if enemy can see them, if not then dont chase
        if (canTransition) {    
            Vector3 rayDir = (playerPos - currentPos).Normalised();
            Vector3 rayPos = GetTransform().GetPosition();

            Vector3 facingDir = GetTransform().GetOrientation() * Vector3(0, 0, -1);
            float angle = acos(Vector3::Dot(rayDir, facingDir) / sqrt(rayDir.LengthSquared() * facingDir.LengthSquared()));
            float angleDeg = Maths::RadiansToDegrees(angle);
            if (angleDeg > fov) return false;                 // if player not within enemy FOV then cant 'see'

            RayCollision closestCollision;
            Ray r = Ray(rayPos, rayDir);
            if (this->world->Raycast(r, closestCollision, true, this, 65.0f)) {
                if ((GameObject*)closestCollision.node == target) {
                    std::cout << "CAN SEE PLAYER - ";
                }
                else {
                    //std::cout << "CANT SEE PLAYER - ";
                    canTransition = false;
                }
            }
        }

        if (canTransition) std::cout << "SWITCH TO CHASE\n";
        return canTransition;
        }));

    stateMachine->AddTransition(new StateTransition(chase, patrol, [&]() -> bool {
        bool canTransition = (target->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Length() > 65.0f;

        // if player within range then check if enemy can see them, if not then stop chasing and switch to patrol
        if (!canTransition) {   
            Vector3 rayDir = (playerPos - currentPos).Normalised();
            Vector3 rayPos = GetTransform().GetPosition();
            RayCollision closestCollision;
            Ray r = Ray(rayPos, rayDir);

            if (this->world->Raycast(r, closestCollision, true, this, 65.0f)) {
                if ((GameObject*)closestCollision.node == target) {
                    //std::cout << "CAN SEE PLAYER - ";
                }
                else {
                    std::cout << "CANT SEE PLAYER - ";
                    canTransition = true;
                }
            }
        }

        if (canTransition) std::cout << "SWITCH TO PATROL\n";
        return canTransition;
        }));
}

EnemyGameObject::~EnemyGameObject() {
    delete stateMachine;
}

void EnemyGameObject::Update(float dt) {    // update position, player position and state machine
    currentPos = GetTransform().GetPosition();
    playerPos = target->GetTransform().GetPosition();
    stateMachine->Update(dt);
}

void EnemyGameObject::OnCollisionBegin(GameObject* otherObject) {
    if (otherObject->GetName() == "Bonus") {
        std::cout << "Enemy collected speed bonus\n";
        speedBonus += 10;
        //world->RemoveGameObject(otherObject);
        otherObject->SetActive(false);
        otherObject->GetTransform().SetPosition(Vector3(0, -20, 0));
        otherObject->GetPhysicsObject()->SetInverseMass(0);
    }
}

void EnemyGameObject::MoveToPosition(Vector3 targetPos) {
    Vector3 direction = (targetPos - currentPos).Normalised();
    GetPhysicsObject()->SetLinearVelocity(direction * speed);
    
    // face towards target position
    float angle = atan2(-direction.x, -direction.z);
    float angleDegrees = Maths::RadiansToDegrees(angle);
    GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), angleDegrees));

}

/*void EnemyGameObject::OnCollisionBegin(GameObject* otherObject) {
    if (otherObject->GetName() == "player") {
        ((PlayerGameObject*)otherObject)->lose = true;
    }
}*/








BTEnemyGameObject::BTEnemyGameObject(PlayerGameObject* player, NavigationGrid* grid, GameWorld* world) {
    this->player = player;
    this->grid = grid;
    this->world = world;

    patrolPoints.push_back(Vector3(105, 5, 185));
    patrolPoints.push_back(Vector3(170, 5, 15));
    patrolPoints.push_back(Vector3(80, 5, 90));
    patrolPoints.push_back(Vector3(55, 5, 85));


    // detect player
    detectPlayer = new BehaviourAction("Detect Player", [&](float dt, BehaviourState state)->BehaviourState {
        if (playerPos.x < 0 || playerPos.z < 0 || playerPosWithY.y > 10) return Failure;    // never detect if outside maze

        if (this->player->holdingHeistItem) return Success;   // if holding heist item then auto detect

        if (timeSinceDetection < 10) return Success;   // auto detect for 10 seconds after last detected - doesnt instantly abandon chase if line of sight lost
        else if (timeSinceDetection < 10.01) std::cout << "lost detection after " << timeSinceDetection << " seconds\n";

        Vector3 rayDir = (playerPosWithY - currentPosWithY).Normalised();    
        Vector3 rayPos = currentPosWithY;
            
        Vector3 facingDir =  GetTransform().GetOrientation() * Vector3(0, 0, -1); 
        float angle = acos(Vector3::Dot(rayDir, facingDir) / sqrt(rayDir.LengthSquared() * facingDir.LengthSquared()));
        float angleDeg = Maths::RadiansToDegrees(angle);
        if (angleDeg > fov) return Failure;                 // if player not within enemy FOV then cant 'see'


        RayCollision closestCollision;  // raycast to player to detect if visible
        Ray r = Ray(rayPos, rayDir);
        if (this->world->Raycast(r, closestCollision, true, this, 65.0f)) {
            if ((PlayerGameObject*)closestCollision.node == this->player) {
                //std::cout << "GOOSE CAN SEE PLAYER - CHASING\n";
                timeSinceDetection = 0;
                return Success;
            }
        }
        return Failure; // if the raycast (last resort) fails then detection fails
        }
    );
    
    // move along path to patrol point
    moveOnPathPatrol = new BehaviourAction("Move On Path Patrol", [&](float dt, BehaviourState state)->BehaviourState {
        if (pathNodes.empty()) return Failure; // if no path

        Vector3 nextPathPos = pathNodes[currentNodeIndex];

        if ((nextPathPos - currentPos).LengthSquared() < 1.0f) { // if close to current node
            if (currentNodeIndex < (pathNodes.size() - 1)) {    // if node isnt the final node, target next node
                currentNodeIndex++;
                nextPathPos = pathNodes[currentNodeIndex];
            }
            else {
                currentPatrolIndex++;
                currentPatrolIndex %= 4;
                return Failure; // if final node reached, return failure so new path can be generated
            }
        }

        MoveToPosition(nextPathPos);  // otherwise move towards next node
        return Ongoing;
        }
    );

    // generate path to patrol point
    pathfindForPatrol = new BehaviourAction("Pathfind For Patrol", [&](float dt, BehaviourState state)->BehaviourState {
        if (Pathfind(patrolPoints[currentPatrolIndex])) return Ongoing;
        else return Failure;
        }
    );

    // check if should sound alarm or not
    needAlarm = new BehaviourAction("Need Alarm", [&](float dt, BehaviourState state)->BehaviourState {
        if (this->player->holdingHeistItem && reachAlarmStatus < 2) {
            if (reachAlarmStatus == 0) {
                std::cout << "pathfinding to alarm\n";
                Pathfind(alarmPos);
                reachAlarmStatus = 1;
            }
            //std::cout << "moving to alarm\n";
            return Failure;
        }
        else return Success;
        }
    );

    // move along path to alarm
    moveToAlarm = new BehaviourAction("Move To Alarm", [&](float dt, BehaviourState state)->BehaviourState {
        if (pathNodes.empty()) return Failure; // if no path

        Vector3 nextPathPos = pathNodes[currentNodeIndex];

        if ((nextPathPos - currentPos).LengthSquared() < 1.0f) { // if close to current node
            if (currentNodeIndex < (pathNodes.size() - 1)) {    // if node isnt the final node, target next node
                currentNodeIndex++;
                nextPathPos = pathNodes[currentNodeIndex];
            }
            else {
                reachAlarmStatus = 2;
                std::cout << "TRIGGER ALARM\n";
                return Success; // if final node reached, return success so can chase player
            }
        }

        MoveToPosition(nextPathPos);  // otherwise move towards next node
        return Ongoing;
        }
    );

    // move along path to player
    moveOnPathToPlayer = new BehaviourAction("Move On Path To Player", [&](float dt, BehaviourState state)->BehaviourState {
        if (pathNodes.empty() || timeSincePathfind > 1) return Failure; // if no path or should pathfind again due to time

        Vector3 nextPathPos = pathNodes[currentNodeIndex];

        if ((nextPathPos - currentPos).LengthSquared() < 1.0f) { // if close to current node
            if (currentNodeIndex < (pathNodes.size() - 1)) {    // if node isnt the final node, target next node
                currentNodeIndex++;
                nextPathPos = pathNodes[currentNodeIndex];
            }
            else {
                return Success; // if final node reached, return success
            }
        }

        MoveToPosition(nextPathPos);  // otherwise move towards next node
        return Ongoing;
        }
    );

    // generate path to player
    pathfindToPlayer = new BehaviourAction("Pathfind To Player", [&](float dt, BehaviourState state)->BehaviourState {
        if (Pathfind(playerPos)) return Ongoing;    
        else return Failure;
        }
    );


    // Move directly towards player (when end of path reached)
    closeMoveToPlayer = new BehaviourAction("Close Move To Player", [&](float dt, BehaviourState state)->BehaviourState {
        if ((playerPos - currentPos).LengthSquared() < 12.0f) return Success;   // caught player, end the game

        MoveToPosition(playerPos);  // otherwise move towards player
        return Ongoing;
        }
    );



    patrolSelector = new BehaviourSelector("Patrol Selector");
    patrolSelector->AddChild(detectPlayer);
    patrolSelector->AddChild(moveOnPathPatrol);
    patrolSelector->AddChild(pathfindForPatrol);

    alarmSelector = new BehaviourSelector("Alarm Selector");
    alarmSelector->AddChild(needAlarm);
    alarmSelector->AddChild(moveToAlarm);

    pathfindPlayerSelector = new BehaviourSelector("Pathfind Player Selector");
    pathfindPlayerSelector->AddChild(moveOnPathToPlayer);
    pathfindPlayerSelector->AddChild(pathfindToPlayer);

    rootSequence = new BehaviourSequence("Root Sequence");
    rootSequence->AddChild(patrolSelector);
    rootSequence->AddChild(alarmSelector);
    rootSequence->AddChild(pathfindPlayerSelector);
    rootSequence->AddChild(closeMoveToPlayer);
}

BTEnemyGameObject::~BTEnemyGameObject() {

}

void BTEnemyGameObject::Update(float dt) {

    currentPos = GetTransform().GetPosition();
    playerPos = player->GetTransform().GetPosition();
    currentPosWithY = currentPos;
    currentPos.y = 0;
    playerPosWithY = playerPos;
    playerPos.y = 0;

    timeSincePathfind += dt;
    timeSinceDetection += dt;

    if (!pathNodes.empty()) {
        for (int i = 1; i < pathNodes.size(); ++i) {    // draws path for debug
            Vector3 a = pathNodes[i - 1];
            Vector3 b = pathNodes[i];

            Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
        }
    }

    if (currentState == Ongoing || currentState == Failure) currentState = rootSequence->Execute(dt);

    if (currentState == Success) player->lose = true;   // player caught so end game
    else if (currentState == Failure) std::cout << "Behaviour Tree error\n";    // error
}

void BTEnemyGameObject::OnCollisionBegin(GameObject* otherObject) {
    if (otherObject->GetName() == "Bonus") {
        std::cout << "Goose collected speed bonus\n";
        speed += 10;
        //world->RemoveGameObject(otherObject);
        otherObject->SetActive(false);
        otherObject->GetTransform().SetPosition(Vector3(0, -20, 0));
        otherObject->GetPhysicsObject()->SetInverseMass(0);
    }
}

void BTEnemyGameObject::MoveToPosition(Vector3 targetPos) {
    Vector3 direction = (targetPos - currentPos).Normalised();
    GetPhysicsObject()->SetLinearVelocity(Vector3(0, GetPhysicsObject()->GetLinearVelocity().y, 0) + direction * speed);

    float angle = atan2(-direction.x, -direction.z);
    float angleDegrees = Maths::RadiansToDegrees(angle);
    GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), angleDegrees));

}

bool BTEnemyGameObject::Pathfind(Vector3 targetPos) { // pathfinds to target position
    pathNodes = {};
    timeSincePathfind = 0;
    currentNodeIndex = 0;

    NavigationPath outPath;
    bool foundPath = grid->FindPath(currentPos, targetPos, outPath); 

    Vector3 pos;
    while (outPath.PopWaypoint(pos)) {  // converts path into Vector3 position nodes
        pathNodes.push_back(pos);
    }

    return foundPath;
}