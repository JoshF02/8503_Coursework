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
    }
}
