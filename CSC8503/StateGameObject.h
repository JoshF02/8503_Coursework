#pragma once
#include "GameObject.h"
#include "PlayerGameObject.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class StateGameObject : public GameObject  {
        public:
            StateGameObject();
            ~StateGameObject();

            void Update(float dt) override;

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);

            StateMachine* stateMachine;
            float counter;
        };


        class EnemyGameObject : public GameObject {
        public:
            EnemyGameObject(PlayerGameObject* target, GameWorld* world, float xMin, float xMax, float zMin, float zMax);
            ~EnemyGameObject();

            void Update(float dt) override;

            void MoveToPosition(Vector3 targetPos);

            void OnCollisionBegin(GameObject* otherObject) override;

            PlayerGameObject* target;
            StateMachine* stateMachine;

        protected:
            std::vector<Vector3> patrolPoints = {};
            int currentPatrolIndex = 0;
            float speed;
            Vector3 currentPos;
            Vector3 playerPos;

            GameWorld* world;
        };


        class BTEnemyGameObject : public GameObject {
        public:
            BTEnemyGameObject(PlayerGameObject* player, NavigationGrid* grid);
            ~BTEnemyGameObject();

            void Update(float dt) override;

        protected:
            PlayerGameObject* player;
            NavigationGrid* grid;
            int currentNodeIndex = 0;
        };
    }
}
