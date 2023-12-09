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

using namespace NCL;
using namespace CSC8503;

class PlayerGameObject : public GameObject {

public:
    PlayerGameObject();

    ~PlayerGameObject();

    void OnCollisionBegin(GameObject* otherObject) override;

    int itemsCollected = 0;
    int itemsLeft = 5;
    int score = 0;
    bool win = false;
    bool lose = false;
};
