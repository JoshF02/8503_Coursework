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
        speed = 30.0f;

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
        speed = 10.0f;
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

void EnemyGameObject::MoveToPosition(Vector3 targetPos) {
    Vector3 direction = (targetPos - currentPos).Normalised();
    GetPhysicsObject()->SetLinearVelocity(direction * speed);
    
    // face towards target position
    float angle = atan2(-direction.x, -direction.z);
    float angleDegrees = Maths::RadiansToDegrees(angle);
    GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), angleDegrees));

}

void EnemyGameObject::OnCollisionBegin(GameObject* otherObject) {
    if (otherObject->GetName() == "player") {
        ((PlayerGameObject*)otherObject)->lose = true;
    }
}








BTEnemyGameObject::BTEnemyGameObject(PlayerGameObject* player, NavigationGrid* grid) {
    this->player = player;
    this->grid = grid;

    patrolPoints.push_back(Vector3(105, 5, 185));
    patrolPoints.push_back(Vector3(170, 5, 15));
    patrolPoints.push_back(Vector3(80, 5, 90));
    patrolPoints.push_back(Vector3(55, 5, 85));


    // detect player - ***NOTE*** change to only if holding heist item / raycast sees them
    detectPlayer = new BehaviourAction("Detect Player", [&](float dt, BehaviourState state)->BehaviourState {
        if (playerPos.x > 0 && playerPos.z > 0 && this->player->shouldTarget) return Success;
        else return Failure;
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

    pathfindPlayerSelector = new BehaviourSelector("Pathfind Player Selector");
    pathfindPlayerSelector->AddChild(moveOnPathToPlayer);
    pathfindPlayerSelector->AddChild(pathfindToPlayer);

    rootSequence = new BehaviourSequence("Root Sequence");
    rootSequence->AddChild(patrolSelector);
    rootSequence->AddChild(pathfindPlayerSelector);
    rootSequence->AddChild(closeMoveToPlayer);
}

BTEnemyGameObject::~BTEnemyGameObject() {

}

void BTEnemyGameObject::Update(float dt) {

    currentPos = GetTransform().GetPosition();
    playerPos = player->GetTransform().GetPosition();
    currentPos.y = 0;
    playerPos.y = 0;

    timeSincePathfind += dt;

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