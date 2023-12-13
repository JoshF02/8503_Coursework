#include "PlayerGameObject.h"
#include "PositionConstraint.h"
#include "PhysicsObject.h"

PlayerGameObject::PlayerGameObject(GameWorld* world) {
    name = "player";
    this->world = world;
}
PlayerGameObject::~PlayerGameObject() {

}

//int counter = 0;

void PlayerGameObject::OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) {
    std::string otherName = otherObject->GetName();

    if (otherName == "key" && !(std::find(alreadyScoredFor.begin(), alreadyScoredFor.end(), otherObject) != alreadyScoredFor.end())) {
        score += 5;
        alreadyScoredFor.push_back(otherObject);

        itemsCollected++;
        itemsLeft--;
    }
    if (otherName == "heistItem" && !(std::find(alreadyScoredFor.begin(), alreadyScoredFor.end(), otherObject) != alreadyScoredFor.end())) {
        score += 20;
        alreadyScoredFor.push_back(otherObject);

        itemsCollected++;
        itemsLeft--;
    }
    if (otherName == "plate" && !(std::find(alreadyScoredFor.begin(), alreadyScoredFor.end(), otherObject) != alreadyScoredFor.end())) {
        score += 5;
        alreadyScoredFor.push_back(otherObject);
    }
    if (otherName == "BTEnemy" || otherName == "Enemy") {
        lose = true;
    }
    
}

void PlayerGameObject::OnCollisionEnd(NCL::CSC8503::GameObject* otherObject) {
    //std::cout << counter << " ONEND\n\n\n";
    if (otherObject->GetName() == "Bonus") {
        std::cout << "Player collected speed bonus\n";
        speedMultiplier += 1.0f;
        //world->RemoveGameObject(otherObject, true);
        //delete otherObject;
        //*otherObject = GameObject();
        //world->RemoveGameObject(this, false);
        otherObject->SetActive(false);
        otherObject->GetTransform().SetPosition(Vector3(0, -20, 0));
        otherObject->GetPhysicsObject()->SetInverseMass(0);
    }
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





KeyGameObject::KeyGameObject(GameObject* doorToOpen, GameWorld* world, bool isHeistItem, PlayerGameObject* player) {
    this->doorToOpen = doorToOpen;
    this->world = world;
    this->isHeistItem = isHeistItem;
    this->player = player;
}

void KeyGameObject::OnCollisionBegin(GameObject* otherObject) { // deletes key and door when key used
    if (!switchActive && otherObject == doorToOpen) {
        switchActive = true;
        this->SetActive(false);

        player->score += 10;

        if (isHeistItem && player) {
            player->score += 50;
            player->win = true;
        }

        world->RemoveGameObject(doorToOpen, false);
        world->RemoveGameObject(this, false);
        std::cout << "KEY USED, DELETING DOOR AND USED KEY\n";
    }

    if (isHeistItem && otherObject == player) {
        PositionConstraint* constraint = new PositionConstraint(player, this, 5.0f);    // constrain heist item to player
        world->AddConstraint(constraint);
        player->holdingHeistItem = true;
    }
}