#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"

#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame()	{
	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
}

void NetworkedGame::StartAsServer() {
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);

	thisServer->RegisterPacketHandler(Received_State, this);
	thisServer->RegisterPacketHandler(String_Message, this);

	StartLevel();
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);
	thisClient->RegisterPacketHandler(String_Message, this);

	StartLevel();
}

void NetworkedGame::UpdateGame(float dt) {
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		if (thisServer) {
			UpdateAsServer(dt);
			thisServer->UpdateServer();
		}
		else if (thisClient) {
			UpdateAsClient(dt);
			thisClient->UpdateClient();
		}
		timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
	}

	/*if (!thisServer && Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {	
		std::cout << "starting server\n";
		StartAsServer();
	}
	if (!thisClient && Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		std::cout << "starting client\n";
		StartAsClient(127,0,0,1);
	}*/

	if (thisClient) {
		TutorialGame::UpdateGame(dt);

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::K)) {
			showScoreTable = !showScoreTable;
		}

		if (showScoreTable) {
			std::string scoreRow = "SERVER HIGH SCORE TABLE";
			Debug::Print(scoreRow, Vector2(30 - scoreRow.length(), 30), Vector4(0, 1, 0, 1));

			int index = 1;
			for (auto i = scoresWithIds.begin(); i != scoresWithIds.end(); ++i) {
				scoreRow = std::to_string(index) + ".  Score: " + std::to_string((int)(*i).x) + "  Client ID: " + std::to_string((int)(*i).y);
				Debug::Print(scoreRow, Vector2(30 - scoreRow.length(), 30 + (index * 10)), Vector4(0, 1, 0, 1));
				index++;
			}
		}
	}
}

bool SortVec2(Vector2 i, Vector2 j) { return (i.x > j.x); }	// orders scores high-low

void NetworkedGame::UpdateAsServer(float dt) {
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) {
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else {
		BroadcastSnapshot(true);
	}

	if (numScores != oldNumScores) {	// save new scores and send out new scores list
		oldNumScores = numScores;
		scoresWithIds.push_back(Vector2(latestScore, latestClient));

		std::sort(scoresWithIds.begin(), scoresWithIds.end(), SortVec2);

		std::string scoresList = "";
		for (auto i = scoresWithIds.begin(); i != scoresWithIds.end(); ++i) {
			scoresList += std::to_string((int)(*i).x) + "," + std::to_string((int)(*i).y) + ",";
		}
		GamePacket* msgFromServer = new StringPacket(scoresList);	
		thisServer->SendGlobalPacket(*msgFromServer);
	}
	if (thisServer->clientCount > oldClientCount) {	// send out new scores list when client connects
		oldClientCount = thisServer->clientCount;

		std::string scoresList = "";
		for (auto i = scoresWithIds.begin(); i != scoresWithIds.end(); ++i) {
			scoresList += std::to_string((int)(*i).x) + "," + std::to_string((int)(*i).y) + ",";
		}
		GamePacket* msgFromServer = new StringPacket(scoresList);
		thisServer->SendGlobalPacket(*msgFromServer);
	}
	else if (thisServer->clientCount < oldClientCount) oldClientCount = thisServer->clientCount; // when client disconnects, update old count
}

void NetworkedGame::UpdateAsClient(float dt) {
	if (player->win && !hasSentScore) {
		GamePacket* msgFromClient = new StringPacket(std::to_string(player->score));
		thisClient->SendPacket(*msgFromClient);
		hasSentScore = true;
	}

	if (!player->win) hasSentScore = false;
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		//TODO - you'll need some way of determining
		//when a player has sent the server an acknowledgement
		//and store the lastID somewhere. A map between player
		//and an int could work, or it could be part of a 
		//NetworkPlayer struct. 
		int playerState = 0;
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			thisServer->SendGlobalPacket(*newPacket);
			delete newPacket;
		}
	}
}

void NetworkedGame::UpdateMinimumState() {
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs) {
		minID = std::min(minID, i.second);
		maxID = std::max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}
}

void NetworkedGame::SpawnPlayer() {

}

void NetworkedGame::StartLevel() {

}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	if (type == String_Message) {
		StringPacket* realPacket = (StringPacket*)payload;
		std::string msg = realPacket->GetStringFromData();
		//std::cout << " received msg (source " << source << "): " << msg << std::endl;

		if (thisServer) {	// if server, record score
			//std::cout << "server received score (source " << source << "): " << msg << std::endl;	
			numScores++;
			latestScore = std::stoi(msg);
			latestClient = source;
		}
		
		else {	// if client, save scores
			if (msg == "") return;

			std::cout << "client received updated score list: " << msg << std::endl;
			scoresWithIds = {};
			std::stringstream ss(msg);

			while (ss.good()) {
				std::string substr;
				getline(ss, substr, ',');	// score
				if (substr == "" || substr == " ") continue;
				int tempScore = std::stoi(substr);

				getline(ss, substr, ',');	// client id
				int tempId = std::stoi(substr);

				scoresWithIds.push_back(Vector2(tempScore, tempId));
			}
		}
	}
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) { //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerNum();

		thisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisClient->SendPacket(newPacket);
	}
}