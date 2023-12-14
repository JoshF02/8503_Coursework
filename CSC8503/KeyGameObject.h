#pragma once
#include "GameObject.h"
#include "Window.h"
#include "Vector3.h"
#include "Quaternion.h"

#include "GameTechRenderer.h"
#include "PhysicsSystem.h"

#include "GameObject.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"

#include "Transform.h"
#include "CollisionVolume.h"
#include "PlayerGameObject.h"

using namespace NCL;
using namespace CSC8503;

class KeyGameObject : public GameObject {

public:
    KeyGameObject(GameObject* doorToOpen = nullptr, GameWorld* world = nullptr, bool isHeistItem = false, PlayerGameObject* player = nullptr);

    //~KeyGameObject();

    void OnCollisionBegin(GameObject* otherObject) override;

    //void OnCollisionStay(GameObject* otherObject) override;

    //void OnCollisionEnd(GameObject* otherObject) override;

    //void Update(float dt) override;

protected:
    bool switchActive = false;
    bool isHeistItem;
    GameObject* doorToOpen;
    GameWorld* world;
    PlayerGameObject* player;
};