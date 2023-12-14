#pragma once
#include "GameObject.h"
#include "PlayerGameObject.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class EnemyGameObject : public GameObject {
        public:
            EnemyGameObject(PlayerGameObject* target, GameWorld* world, float xMin, float xMax, float zMin, float zMax);
            ~EnemyGameObject();

            void Update(float dt) override;

            void OnCollisionBegin(GameObject* otherObject) override;

            void MoveToPosition(Vector3 targetPos);

            PlayerGameObject* target;
            StateMachine* stateMachine;

        protected:
            std::vector<Vector3> patrolPoints = {};
            int currentPatrolIndex = 0;
            float speed = 30;
            float speedBonus = 0;
            Vector3 currentPos;
            Vector3 playerPos;
            GameWorld* world;
            float fov = 70; // angle of area in front of enemy that it can detect player within
        };
    }
}
