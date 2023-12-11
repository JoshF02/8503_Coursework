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
}

BTEnemyGameObject::~BTEnemyGameObject() {

}

void BTEnemyGameObject::Update(float dt) {

    // Pathfinding test
    std::vector<Vector3> testNodes = {};
    NavigationPath outPath;
    Vector3 startPos = GetTransform().GetPosition();
    Vector3 endPos = player->GetTransform().GetPosition();
    startPos.y = 0;
    endPos.y = 0;

    bool found = false;
    if (endPos.x > 0 && endPos.z > 0) {
        found = grid->FindPath(startPos, endPos, outPath);

        Vector3 pos;
        while (outPath.PopWaypoint(pos)) {
            testNodes.push_back(pos);
        }

        for (int i = 1; i < testNodes.size(); ++i) {
            Vector3 a = testNodes[i - 1];
            Vector3 b = testNodes[i];

            Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
        }
    }

    /*if (!testNodes.empty()) {
        Vector3 nodeDir = testNodes[currentNodeIndex] - startPos;

        if (nodeDir.LengthSquared() < 1.0f) {
            currentNodeIndex++;
            nodeDir = testNodes[currentNodeIndex] - startPos;
        }

        GetPhysicsObject()->AddForce(nodeDir * 100.0f);
    }*/
    if (found && testNodes.size() > 1) {
        //Vector3 nodeDir = testNodes[1] - startPos;
        //GetPhysicsObject()->AddForce(nodeDir * 10.0f);

        /*float alpha = dt;
        Vector3 lerp = (testNodes[1] * alpha) + (startPos * (1 - alpha));
        lerp.y = GetTransform().GetPosition().y;
        GetTransform().SetPosition(lerp);*/

        Vector3 direction = (testNodes[1] - startPos).Normalised();
        GetPhysicsObject()->SetLinearVelocity(direction * 10);

        // face towards target position
        float angle = atan2(-direction.x, -direction.z);
        float angleDegrees = Maths::RadiansToDegrees(angle);
        GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), angleDegrees));
    }
}