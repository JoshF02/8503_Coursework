#include "PlayerGameObject.h"
#include "PositionConstraint.h"
#include "PhysicsObject.h"
#include "SwitchGameObject.h"
#include <iostream>

SwitchGameObject::SwitchGameObject(bool onTimer, GameObject* doorToOpen, PlayerGameObject* player) {
    isOnTimer = onTimer;
    this->doorToOpen = doorToOpen;
    this->player = player;

    doorOriginalY = doorToOpen->GetTransform().GetPosition().y;
}

void SwitchGameObject::OnCollisionBegin(GameObject* otherObject) {
    //std::cout << "ONBEGIN\n";
    timeSinceLastEnter = 0;

    if (!switchActive && otherObject->GetName() != "Enemy") {
        std::cout << "ISONTIMER=" << isOnTimer << " ACTIVATING\n";
        switchActive = true;
    }
}

void SwitchGameObject::Update(float dt) {
    //std::cout << "UPDATE\n";
    if (switchActive && isOnTimer) {
        timeSinceLastEnter += dt;

        if (timeSinceLastEnter > 0.5f) {
            std::cout << "TIMER REACHED, DEACTIVATING\n";
            timeSinceLastEnter = 0;
            switchActive = false;
        }
    }

    if (switchActive) {
        //std::cout << "TRIGGERING DOOR OPEN\n";
        Vector3 currentDoorPos = doorToOpen->GetTransform().GetPosition();
        if (currentDoorPos.y < (doorOriginalY + amountToOpen)) {
            doorToOpen->GetTransform().SetPosition(currentDoorPos + Vector3(0, openSpeed, 0));  // move up if y less than max y
        }

    }
    else {
        Vector3 currentDoorPos = doorToOpen->GetTransform().GetPosition();
        if (currentDoorPos.y > (doorOriginalY)) {
            doorToOpen->GetTransform().SetPosition(currentDoorPos - Vector3(0, openSpeed, 0));  // move down if y more than original y
        }
    }

}

/*void SwitchGameObject::OnCollisionStay(GameObject* otherObject) {
    std::cout << "STAY\n";
}*/