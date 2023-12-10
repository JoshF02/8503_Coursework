#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "NavigationGrid.h"


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world = new GameWorld();
#ifdef USEVULKAN
	renderer = new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = true;
	physics->UseGravity(useGravity);
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("coin.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	if (menu) {
		InitCamera();

		if (!gameHasStarted) {	// loaded into menu upon opening game
			Debug::Print("1. Start Game ", Vector2(30, 40), Debug::GREEN);
		}
		else {	// brought up menu by pausing
			Debug::Print("1. Restart Game ", Vector2(30, 40), Debug::GREEN);
			Debug::Print("2. Unpause Game ", Vector2(30, 50), Debug::GREEN);
		}
		Debug::Print("Exit - Press ESC", Vector2(30, 60), Debug::GREEN);
		
		renderer->Update(dt);
		renderer->Render();

		Debug::UpdateRenderables(dt);
		return;
	}

	if (!inSelectionMode) {
		world->GetMainCamera().UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + Quaternion::Quaternion(Matrix4::Rotation(yaw, Vector3(0, 1, 0))) * lockedOffset;	// rotate camera around player

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		//world->GetMainCamera().SetPitch(angles.x);	// let player control pitch with mouse
		world->GetMainCamera().SetYaw(angles.y);

		// move picked up object with player
		if (pickedUpObj != nullptr) {

			if (!pickedUpObj->IsActive()) {
				std::cout << "PICKED UP OBJ NOT ACTIVE SO REMOVING\n";	
				pickedUpObj = nullptr;
			}
			else {
				pickedUpObj->GetTransform().SetPosition(objPos + Quaternion::Quaternion(Matrix4::Rotation(yaw, Vector3(0, 1, 0))) * pickedUpOffset);
			}
		}
	}

	UpdateKeys();

	/*if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}*/

	/*RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}*/

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::B)) {	// TEST GAME OVER
		player->itemsLeft--;
		player->itemsCollected++;

		if (player->itemsLeft == 0) {
			player->win = true;
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::N)) {	// TEST GAME OVER
		player->lose = true;
	}


	if (!player->win && !player->lose) {
		UpdateScoreAndTimer(dt);
	}
	else {
		world->UpdateWorld(dt);
		renderer->Update(dt);
		physics->Update(dt);
		EndGame();
		renderer->Render();
		return;
	}




	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	if (testStateObject) {
		testStateObject->Update(dt);
	}

	//SelectObject();
	//MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	/*if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}*/

	/*if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}*/
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	/*Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);*/

	Vector3 fwdAxis = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);	// using player object's axes instead
	Vector3 rightAxis = Vector3::Cross(fwdAxis, Vector3(0, 1, 0));

	//std::cout << fwdAxis << "VS\n" << Vector3::Cross(Vector3(0, 1, 0), Vector3(camWorld.GetColumn(0))) << "\n";
	//std::cout << rightAxis << "VS\n" << Vector3(camWorld.GetColumn(0)) << "\n\n";

	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis * 20);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis * 20);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		selectionObject->GetPhysicsObject()->AddForce(-rightAxis * 20);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		selectionObject->GetPhysicsObject()->AddForce(rightAxis * 20);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE)) {	// only allow jump if on ground
		
		RayCollision closestCollision;
		Vector3 rayDir = Vector3(0, -1, 0);
		Vector3 rayPos = selectionObject->GetTransform().GetPosition();
		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject, 1.5f)) {
			//std::cout << "CLOSE TO GROUND, ALLOWING JUMP\n";
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 200, 0));
		}
	}

	/*if (Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -200, 0));
	}*/

	// rotate player with mouse
	yaw -= controller.GetNamedAxis("XLook");

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	selectionObject->GetTransform().SetOrientation(Quaternion::Quaternion(Matrix4::Rotation(yaw, Vector3(0, 1, 0))));

	// pick up item in front of player
	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::E) && !pickedUpObj) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);


		/*GameObject* player = selectionObject;

		yaw = controller.GetNamedAxis("XLook");

		if (yaw < 0) {
			yaw += 360.0f;
		}
		if (yaw > 360.0f) {
			yaw -= 360.0f;
		}

		Quaternion facingDir = player->GetTransform().GetOrientation();

		selectionObject->GetTransform().SetOrientation(facingDir + 
			Quaternion::Quaternion(Matrix4::Rotation(yaw, Vector3(0, 1, 0))));*/


		if (world->Raycast(r, closestCollision, true, selectionObject, 10.0f)) {

			float inverseMass = ((GameObject*)closestCollision.node)->GetPhysicsObject()->GetInverseMass();
			if (inverseMass != 0) {	// dont pick up static objects
				pickedUpObj = (GameObject*)closestCollision.node;
				pickedUpObj->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
				oldInverseMass = inverseMass;	// saves inverse mass of object
				pickedUpObj->GetPhysicsObject()->SetInverseMass(0);	// sets inverse mass of object to 0 so forces arent applied to it
				pickedUpObj->GetPhysicsObject()->ClearForces();

				if (pickedUpObj->GetName() == "key") {
					player->OnCollisionBegin(pickedUpObj);	// gives score for picking up, as if you collided with it
				}
			}
		}
	}

	// drop item
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::R) && pickedUpObj) {
		pickedUpObj->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));	// resets colour and inverse mass before dropping
		pickedUpObj->GetPhysicsObject()->SetInverseMass(oldInverseMass);
		pickedUpObj = nullptr;
	}

	// throw item
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F) && pickedUpObj) {
		pickedUpObj->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
		pickedUpObj->GetPhysicsObject()->SetInverseMass(oldInverseMass);
		pickedUpObj->GetPhysicsObject()->AddForce(fwdAxis * 500);			// doesnt always apply force properly
		pickedUpObj = nullptr;
	}

	// rotate item
	if (pickedUpObj) {
		float torque = Window::GetMouse()->GetWheelMovement() * 100.0f;
		pickedUpObj->GetPhysicsObject()->AddTorque(Vector3(torque, 0, 0));
	}
}

void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	menu = false;
	timer = maxTimer;

	world->ClearAndErase();
	physics->Clear();

	player = AddPlayerToWorld(Vector3(100, 0.02f, -100));	// adds player to world
	InitPlayer();
	yaw = 0;

	InitDefaultFloor();

	// External Walls
	AddCubeToWorld(Vector3(0, 10, -200), Vector3(200, 10, 1), 0);
	AddCubeToWorld(Vector3(0, 10, 200), Vector3(200, 10, 1), 0);
	AddCubeToWorld(Vector3(-200, 10, 0), Vector3(1, 10, 200), 0);
	AddCubeToWorld(Vector3(200, 10, 0), Vector3(1, 10, 200), 0);


	// Internal Walls
	AddCubeToWorld(Vector3(-75, 5, 1), Vector3(75, 5, 1), 0);	// centre walls
	AddCubeToWorld(Vector3(-185, 5, 1), Vector3(15, 5, 1), 0);
	AddCubeToWorld(Vector3(-100, 5, 46), Vector3(1, 5, 44), 0);	// walls between zones 2 and 3
	AddCubeToWorld(Vector3(-100, 5, 154.5), Vector3(1, 5, 44.5), 0);


	// Zone 1
	GameObject* zone1Door = AddCubeToWorld(Vector3(-160, 5, 1), Vector3(10, 5, 1), 0);
	zone1Door->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	AddPressurePlateToWorld(Vector3(-160, 0.01f, -70), false, zone1Door);
	AddEnemyToWorld(Vector3(-10, 2.5, -100), -10, -180, -10, -180);
	AddEnemyToWorld(Vector3(-30, 2.5, -70), -30, -160, -30, -160);
	AddEnemyToWorld(Vector3(10, 2.5, -100), 10, 180, -10, -180);
	AddEnemyToWorld(Vector3(30, 2.5, -70), 30, 160, -30, -160);


	// Zone 2
	GameObject* zone2Door = AddCubeToWorld(Vector3(-100, 5, 100), Vector3(1, 5, 10), 0);
	zone2Door->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	AddKeyToWorld(Vector3(-170, 2, -160), zone2Door);	// put at end of bridge in zone 1
	AddOBBToWorld(Vector3(-170, 2, 150), Vector3(2, 2, 2));
	AddOBBToWorld(Vector3(-180, 2, 150), Vector3(2, 2, 2));
	AddOBBToWorld(Vector3(-170, 2, 140), Vector3(2, 2, 2));
	AddOBBToWorld(Vector3(-180, 2, 140), Vector3(2, 2, 2));
	AddOBBToWorld(Vector3(-170, 6, 150), Vector3(2, 2, 2));
	AddOBBToWorld(Vector3(-170, 10, 150), Vector3(2, 2, 2));
	AddOBBToWorld(Vector3(-150, 4, 60), Vector3(10, 4, 2), 0.05f);


	// Zone 3
	GameObject* zone3Door = AddCubeToWorld(Vector3(5, 5, 170), Vector3(5, 5, 10), 0);
	zone3Door->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	AddPressurePlateToWorld(Vector3(-50, 0.01f, 70), true, zone3Door);

	
	// Zone 4 (Maze)
	GameObject* startingArea = AddCubeToWorld(Vector3(100, 0.01f, -100), Vector3(10, 0.01f, 10), 0);
	startingArea->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
	AddKeyToWorld(Vector3(170, 2, 160), startingArea, true);
	AddMazeToWorld();

	//InitTestingObjs();
}

void TutorialGame::InitTestingObjs() {
	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);

	//BridgeConstraintTest();

	InitGameExamples();

	for (int i = 0; i < 4; ++i) {
		bool onTimer = (rand() % 2 > 0.5) ? true : false;
		GameObject* door = AddCubeToWorld(Vector3(30 * i, 20, 60), Vector3(5, 5, 5), 0);
		AddPressurePlateToWorld(Vector3(30 * i, 11, 40), onTimer, door);
	}

	GameObject* door = AddCubeToWorld(Vector3(-50, 10, 90), Vector3(5, 10, 5), 0);
	AddKeyToWorld(Vector3(-50, 0, 60), door);

	AddSphereToWorld(Vector3(-50, 0, 40), 10, 0);

	testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
}

void TutorialGame::InitPlayer() { 	// gives control + camera to player
	lockedObject = player;
	selectionObject = player;
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

// Adds immovable pressure plate
SwitchGameObject* TutorialGame::AddPressurePlateToWorld(const Vector3& position, bool onTimer, GameObject* door) {
	SwitchGameObject* plate = new SwitchGameObject(onTimer, door);
	std::string name = "plate";
	plate->SetName(name);

	Vector3 plateSize = Vector3(10, 0.01f, 10);
	AABBVolume* volume = new AABBVolume(plateSize);
	plate->SetBoundingVolume((CollisionVolume*)volume);
	plate->GetTransform()
		.SetScale(plateSize * 2)
		.SetPosition(position);

	plate->SetRenderObject(new RenderObject(&plate->GetTransform(), cubeMesh, basicTex, basicShader));
	plate->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
	plate->SetPhysicsObject(new PhysicsObject(&plate->GetTransform(), plate->GetBoundingVolume()));

	plate->GetPhysicsObject()->SetInverseMass(0);
	plate->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(plate);

	return plate;
}

// Adds key
KeyGameObject* TutorialGame::AddKeyToWorld(const Vector3& position, GameObject* door, bool isHeistItem) {
	KeyGameObject* key = new KeyGameObject(door, world, isHeistItem, player);
	std::string name = "key";
	key->SetName(name);

	Vector3 keySize = Vector3(2, 2, 2);
	AABBVolume* volume = new AABBVolume(keySize);
	key->SetBoundingVolume((CollisionVolume*)volume);
	key->GetTransform()
		.SetScale(keySize * 2)
		.SetPosition(position);

	key->SetRenderObject(new RenderObject(&key->GetTransform(), cubeMesh, basicTex, basicShader));
	key->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
	key->SetPhysicsObject(new PhysicsObject(&key->GetTransform(), key->GetBoundingVolume()));

	key->GetPhysicsObject()->SetInverseMass(10);
	key->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(key);

	return key;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOBBToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

PlayerGameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	PlayerGameObject* character = new PlayerGameObject();
	SphereVolume* volume = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

EnemyGameObject* TutorialGame::AddEnemyToWorld(const Vector3& position, float xMin, float xMax, float zMin, float zMax) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	EnemyGameObject* character = new EnemyGameObject(player, world, xMin, xMax, zMin, zMax);
	//character->SetName("EnemyPlayer");

	//AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	CapsuleVolume* volume = new CapsuleVolume(0.9f * meshSize, 0.7f * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();
	//TriggerGameObject* apple = new TriggerGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.2f, 0.2f, 0.2f))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.2f, 0.2f, 0.2f))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));	// floor has height of 2 so reaches y=0
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 15, 0));	// collision volumes dont match meshes well so dont sit on floor properly
	AddEnemyToWorld(Vector3(-10, 2.5, -100), -10, -180, -10, -180);
	AddEnemyToWorld(Vector3(-30, 2.5, -70), -30, -160, -30, -160);

	AddBonusToWorld(Vector3(10, 15, 0));

	AddCubeToWorld(Vector3(5, 50, 0), Vector3(1, 1, 1));
	AddCubeToWorld(Vector3(5, 15, 0), Vector3(1, 1, 1));

	AddOBBToWorld(Vector3(15, 15, 0), Vector3(1, 1, 1));
	AddOBBToWorld(Vector3(20, 15, 0), Vector3(1, 1, 1));
	AddOBBToWorld(Vector3(20, 25, 0), Vector3(1, 1, 1));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
				//AddOBBToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}


void TutorialGame::BridgeConstraintTest() {
	//implemented
	Vector3 cubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5;
	int numLinks = 10;
	float maxDistance = 30;
	float cubeDistance = 20;

	Vector3 startPos = Vector3(0, 100.0f, 0);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}


void TutorialGame::AddMazeToWorld() {
	NavigationGrid grid("TestGrid2.txt");

	for (int y = 0; y < grid.gridHeight; ++y) {
		for (int x = 0; x < grid.gridWidth; ++x) {
			GridNode& n = grid.allNodes[(grid.gridWidth * y) + x];

			// x = 120 in decimal, wall
			if (n.type == 120) AddCubeToWorld(n.position + Vector3(5, 5, 5), { (float)grid.nodeSize / 2,(float)grid.nodeSize / 2,(float)grid.nodeSize / 2 }, 0);

			// c = 99 coin
			//if (n.type == 99) AddBonusToWorld(n.position + Vector3(0, 5, 0));

			// e = 101 enemy
			//if (n.type == 101) AddEnemyToWorld(n.position + Vector3(0, 5, 0));

			// i = 105 state object	
			//if (n.type == 105) AddStateObjectToWorld(n.position + Vector3(0, 5, 0));

			// p = 112 player
			//if (n.type == 112) AddPlayerToWorld(n.position + Vector3(0, 5, 0));
		}
	}
}

void TutorialGame::UpdateScoreAndTimer(float dt) {
	timer -= dt;
	int minutes = floor(timer / 60.0f);
	int seconds = std::round(std::fmod(timer, 60.0f));
	if (seconds == 60) seconds = 0, minutes++;
	Vector4 timerColor = timer <= 20.0f ? Debug::RED : Debug::YELLOW;
	std::string time = "Time: " + std::to_string(minutes) + ":" + std::to_string(seconds);
	Debug::Print(time, Vector2(90 - time.length(), 5), timerColor);
	if (timer <= 0.0f) {
		timer = 0.0f;
		player->lose = true;
	}

	std::string score = "Score = " + std::to_string(player->score);
	Debug::Print(score, Vector2(90 - time.length(), 10), timerColor);

	std::string itemsLeft = "Items Left = " + std::to_string(player->itemsLeft);
	Debug::Print(itemsLeft, Vector2(90 - time.length() - 9.5, 15), timerColor);

	std::string itemsCollected = "(" + std::to_string(player->itemsCollected) + " Collected)";
	Debug::Print(itemsCollected, Vector2(90 - time.length() - 6, 20), timerColor);

	std::string ComeBackMenu = "F3 : Pause Game ";
	Debug::Print(ComeBackMenu, Vector2(0, 10), Debug::BLUE);

}

void TutorialGame::EndGame() {

	InitCamera();

	std::string wonOrLost = "";
	if (player->lose) wonOrLost = "You Lost...";
	if (player->win) wonOrLost = "You Won!";
	Debug::Print(wonOrLost, Vector2(30, 40));

	std::string score = "Score = ";
	score.append(std::to_string(player->score));
	score.append(";");
	Debug::Print(score, Vector2(30, 50));


	std::string itemLeft = "Items Left = ";
	itemLeft.append(std::to_string(player->itemsLeft));
	itemLeft.append(";");
	Debug::Print(itemLeft, Vector2(30, 60));

	std::string text = "Play Again(F3);";

	Debug::Print(text, Vector2(30, 70));

	text = "Exit (ESC);";
	Debug::Print(text, Vector2(30, 80));

	gameHasStarted = false;
}


