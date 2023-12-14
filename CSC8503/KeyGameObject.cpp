#include "PlayerGameObject.h"
#include "PositionConstraint.h"
#include "PhysicsObject.h"
#include "KeyGameObject.h"
#include <iostream>

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