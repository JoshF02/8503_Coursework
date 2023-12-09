#include "PlayerGameObject.h"

PlayerGameObject::PlayerGameObject() {

}
PlayerGameObject::~PlayerGameObject() {

}
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