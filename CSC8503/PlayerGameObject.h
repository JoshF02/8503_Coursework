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
    PlayerGameObject();

    ~PlayerGameObject();

    void OnCollisionBegin(GameObject* otherObject) override;

    //void OnCollisionStay(GameObject* otherObject) override;

    void OnCollisionEnd(GameObject* otherObject) override;

    //void Update(float dt) override;;

    int itemsCollected = 0;
    int itemsLeft = 2;
    int score = 0;
    bool win = false;
    bool lose = false;

    bool holdingHeistItem = false;  // MAKE THIS TRUE IF HOLDING HEIST ITEM

    std::vector<GameObject*> alreadyScoredFor = {};
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
