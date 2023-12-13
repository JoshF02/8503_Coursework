#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "PlayerGameObject.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			void InitMenu() { menu = true; };
			void DisableMenu() { menu = false; };

			void InitWorld();
			void InitTestingObjs();
			void InitCamera();
			void InitPlayer();

			bool gameHasStarted = false;

		protected:
			void InitialiseAssets();

			void UpdateKeys();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on).
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddOBBToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			PlayerGameObject* AddPlayerToWorld(const Vector3& position);
			EnemyGameObject* AddEnemyToWorld(const Vector3& position, float xMin, float xMax, float zMin, float zMax);
			BTEnemyGameObject* AddBTEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddPlaneToWorld(const Vector3& position);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			void BridgeConstraintTest();

			void AddMazeToWorld();

			void UpdateScoreAndTimer(float dt);
			void EndGame();

			SwitchGameObject* AddPressurePlateToWorld(const Vector3& position, bool onTimer, GameObject* door, float scaleFactor = 1.0f);
			KeyGameObject* AddKeyToWorld(const Vector3& position, GameObject* door, bool isHeistItem = false);

#ifdef USEVULKAN
			GameTechVulkanRenderer* renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem* physics;
			GameWorld* world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;
			bool menu;

			float forceMagnitude;
			float yaw;

			GameObject* selectionObject = nullptr;

			Mesh* capsuleMesh = nullptr;
			Mesh* cubeMesh = nullptr;
			Mesh* sphereMesh = nullptr;

			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;

			//Coursework Meshes
			Mesh* charMesh = nullptr;
			Mesh* enemyMesh = nullptr;
			Mesh* bonusMesh = nullptr;
			Mesh* gooseMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			Vector3 lockedOffset = Vector3(0, 8, 14);
			Vector3 pickedUpOffset = Vector3(0, 4, -8);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;
			GameObject* pickedUpObj = nullptr;

			float oldInverseMass = 0;

			float timer = 0;
			float maxTimer = 300.0f;

			PlayerGameObject* player = nullptr;

			NavigationGrid* grid;

			bool freeRotation = false;
		};
	}
}

