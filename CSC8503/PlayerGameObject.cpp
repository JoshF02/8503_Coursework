#include "PlayerGameObject.h"

PlayerGameObject::PlayerGameObject() {

}
PlayerGameObject::~PlayerGameObject() {

}

//int counter = 0;

void PlayerGameObject::OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) {

    if (name == "player" && otherObject->GetName() == "coinTools") {
        otherObject->SetActive(false);
        otherObject->SetBoundingVolume(nullptr);
        score += 10;
        itemsCollected++;
        itemsLeft--;

        if (itemsLeft == 0) {
            win = true;
        }
    }

    //std::cout << counter << " ONBEGIN\n";

    /*if (name == "player" && otherObject->GetName() == "sphereTools") {
        otherObject->SetActive(false);
        otherObject->SetBoundingVolume(nullptr);
        score += 10;
        itemsCollected++;
        itemsLeft--;
    }*/
    /*if (name == "player" && otherObject->GetName() == "keyTools") {
        otherObject->SetIsActive(false);
        otherObject->SetBoundingVolume(nullptr);
        keyNum++;
    }*/
}

void PlayerGameObject::OnCollisionEnd(NCL::CSC8503::GameObject* otherObject) {
    //std::cout << counter << " ONEND\n\n\n";
}


/*TriggerGameObject::TriggerGameObject() {
    isTrigger = true;
    //renderObject = nullptr;
}

void TriggerGameObject::OnCollisionBegin(GameObject* otherObject) {
    std::cout << counter << " ONBEGIN\n";
    counter++;
    triggerActive = true;
}

void TriggerGameObject::OnCollisionEnd(GameObject* otherObject) {
    std::cout << counter << " ONEND\n";
    counter++;
    triggerActive = false;
}*/

PressurePlateGameObject::PressurePlateGameObject(bool onTimer) {
    isOnTimer = onTimer;
}

void PressurePlateGameObject::OnCollisionBegin(GameObject* otherObject) {
    //std::cout << "ONBEGIN\n";
    timeSinceLastEnter = 0;

    if (!pressurePlateActive) {
        std::cout << "ISONTIMER=" << isOnTimer << " ACTIVATING\n";
        pressurePlateActive = true;
    }
}

void PressurePlateGameObject::Update(float dt) {
    //std::cout << "UPDATE\n";
    if (pressurePlateActive && isOnTimer) {
        timeSinceLastEnter += dt;

        if (timeSinceLastEnter > 0.5f) {
            std::cout << "TIMER REACHED, DEACTIVATING\n";
            timeSinceLastEnter = 0;
            pressurePlateActive = false;
        }
    }
}

/*void PressurePlateGameObject::OnCollisionStay(GameObject* otherObject) {
    std::cout << "STAY\n";
}*/