#include "BTEnemyGameObject.h"
#include "PhysicsObject.h"
#include <iostream>

using namespace NCL;
using namespace CSC8503;


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

        Vector3 facingDir = GetTransform().GetOrientation() * Vector3(0, 0, -1);
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
        player->itemsLeft--;

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