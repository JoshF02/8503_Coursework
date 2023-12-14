#include "PlayerGameObject.h"
#include "PositionConstraint.h"
#include "PhysicsObject.h"

PlayerGameObject::PlayerGameObject(GameWorld* world) {
    name = "player";
    this->world = world;
}
PlayerGameObject::~PlayerGameObject() {

}

void PlayerGameObject::OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) {
    std::string otherName = otherObject->GetName();

    if (otherName == "key" && !(std::find(alreadyScoredFor.begin(), alreadyScoredFor.end(), otherObject) != alreadyScoredFor.end())) {
        score += 5;
        alreadyScoredFor.push_back(otherObject);
    }
    if (otherName == "heistItem" && !(std::find(alreadyScoredFor.begin(), alreadyScoredFor.end(), otherObject) != alreadyScoredFor.end())) {
        score += 20;
        alreadyScoredFor.push_back(otherObject);
    }
    if (otherName == "plate" && !(std::find(alreadyScoredFor.begin(), alreadyScoredFor.end(), otherObject) != alreadyScoredFor.end())) {
        score += 5;
        alreadyScoredFor.push_back(otherObject);
    }
    if (otherName == "BTEnemy" || otherName == "Enemy") {
        lose = true;
    }
    if (otherName == "Bonus" && !(std::find(alreadyScoredFor.begin(), alreadyScoredFor.end(), otherObject) != alreadyScoredFor.end())) {
        std::cout << "Player collected speed bonus\n";
        score += 2;
        alreadyScoredFor.push_back(otherObject);

        itemsCollected++;
        itemsLeft--;
        speedMultiplier += 1.0f;
        
        otherObject->SetActive(false);
        otherObject->GetTransform().SetPosition(Vector3(0, -20, 0));
        otherObject->GetPhysicsObject()->SetInverseMass(0);
    }
    
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
