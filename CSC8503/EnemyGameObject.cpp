#include "EnemyGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include <iostream>

using namespace NCL;
using namespace CSC8503;

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
        target->itemsLeft--;

        //world->RemoveGameObject(otherObject);
        otherObject->SetActive(false);
        otherObject->GetTransform().SetPosition(Vector3(0, -20, 0));
        otherObject->GetPhysicsObject()->SetInverseMass(0);
    }

    else if (otherObject->GetName() == "sphere" || otherObject->GetName() == "obb") {   // throw props at enemies to destroy them
        if (otherObject->GetPhysicsObject()->GetLinearVelocity().Length() > 20) {
            //std::cout << otherObject->GetPhysicsObject()->GetLinearVelocity().Length() << " Enemy killed with thrown object\n";
            target->score += 5;
            world->RemoveGameObject(this, false);
            world->RemoveGameObject(otherObject, false);
        }
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