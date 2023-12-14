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
//#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

class PlayerGameObject : public GameObject {

public:
    PlayerGameObject(GameWorld* world = nullptr);

    ~PlayerGameObject();

    void OnCollisionBegin(GameObject* otherObject) override;

    //void OnCollisionStay(GameObject* otherObject) override;

    void OnCollisionEnd(GameObject* otherObject) override;

    //void Update(float dt) override;;

    int itemsCollected = 0;
    int itemsLeft = 4;
    int score = 0;
    bool win = false;
    bool lose = false;

    bool holdingHeistItem = false;  

    std::vector<GameObject*> alreadyScoredFor = {};

    float speedMultiplier = 1.0f;
    GameWorld* world;
};

/*class TriggerGameObject : public GameObject {

public:
    TriggerGameObject();

    //~TriggerGameObject();

    void OnCollisionBegin(GameObject* otherObject) override;

    //void OnCollisionStay(GameObject* otherObject) override;

    void OnCollisionEnd(GameObject* otherObject) override;

    //void Update(float dt) override;

protected:
    bool triggerActive = false;
    int counter = 0;
};*/

