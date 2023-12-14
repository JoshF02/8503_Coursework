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

class SwitchGameObject : public GameObject {

public:
    SwitchGameObject(bool onTimer, GameObject* doorToOpen = nullptr, PlayerGameObject* player = nullptr);

    //~SwitchGameObject();

    void OnCollisionBegin(GameObject* otherObject) override;

    //void OnCollisionStay(GameObject* otherObject) override;

    //void OnCollisionEnd(GameObject* otherObject) override;

    void Update(float dt) override;

protected:
    bool switchActive = false;
    bool isOnTimer;
    GameObject* doorToOpen;
    float timeSinceLastEnter = 0;
    float doorOriginalY;
    float amountToOpen = 15.0f;
    float openSpeed = 0.1f;
    PlayerGameObject* player;
};