#pragma once
#include "GameObject.h"
#include "PlayerGameObject.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

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

            //void OnCollisionBegin(GameObject* otherObject) override;

            PlayerGameObject* target;
            StateMachine* stateMachine;

        protected:
            std::vector<Vector3> patrolPoints = {};
            int currentPatrolIndex = 0;
            float speed;
            Vector3 currentPos;
            Vector3 playerPos;
            GameWorld* world;
            float fov = 70; // angle of area in front of enemy that it can detect player within
        };


        class BTEnemyGameObject : public GameObject {
        public:
            BTEnemyGameObject(PlayerGameObject* player, NavigationGrid* grid, GameWorld* world);
            ~BTEnemyGameObject();

            void Update(float dt) override;

            void MoveToPosition(Vector3 targetPos);
            bool Pathfind(Vector3 targetPos);

        protected:
            PlayerGameObject* player;
            NavigationGrid* grid;
            float timeSincePathfind = 0;
            Vector3 currentPos;
            Vector3 playerPos;
            float speed = 10;
            std::vector<Vector3> pathNodes = {};
            int currentNodeIndex = 0;
            std::vector<Vector3> patrolPoints = {};
            int currentPatrolIndex = 0;

            BehaviourState currentState = Ongoing;

            BehaviourSequence* rootSequence;

            BehaviourSelector* patrolSelector;
            BehaviourAction* detectPlayer;
            BehaviourAction* moveOnPathPatrol;
            BehaviourAction* pathfindForPatrol;

            BehaviourSelector* alarmSelector;
            BehaviourAction* needAlarm;
            BehaviourAction* moveToAlarm;

            BehaviourSelector* pathfindPlayerSelector;
            BehaviourAction* moveOnPathToPlayer;
            BehaviourAction* pathfindToPlayer;

            BehaviourAction* closeMoveToPlayer;

            Vector3 alarmPos = Vector3(135, 0, 135);
            int reachAlarmStatus = 0;   // 0 for no path, 1 for path but hasnt reached, 2 for has reached
            GameWorld* world;

            Vector3 currentPosWithY;
            Vector3 playerPosWithY;

            float fov = 70; // angle of area in front of enemy that it can detect player within

            float timeSinceDetection = 10;
        };
    }
}
