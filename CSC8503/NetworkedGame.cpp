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
		if (thisClient) {
			UpdateAsClient(dt);
			thisClient->UpdateClient();
		}
		timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
	}

	/*if (!thisServer && Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {	// move into initialisation
		std::cout << "starting server\n";
		StartAsServer();
	}
	if (!thisClient && Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		std::cout << "starting client\n";
		StartAsClient(127,0,0,1);
	}*/

	if (!thisServer) TutorialGame::UpdateGame(dt);
}

void NetworkedGame::UpdateAsServer(float dt) {
	//std::cout << "updating as server\n";
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) {
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else {
		BroadcastSnapshot(true);
	}

	if (numScores != oldNumScores) {
		//GamePacket* msgFromServer = new StringPacket(" Number of scores recorded increased to " + std::to_string(numScores));
		//thisServer->SendGlobalPacket(*msgFromServer);

		oldNumScores = numScores;
		scores.push_back(latestScore);
		clientThatGotScore.push_back(latestClient);
		//std::cout << "score of " << latestScore << " saved\n";

		std::string scoresList = "";
		int index = 0;
		for (auto i = scores.begin(); i != scores.end(); ++i) {
			scoresList += std::to_string(*i) + ": " + std::to_string(clientThatGotScore[index]) + ", ";
			index++;
		}
		GamePacket* msgFromServer = new StringPacket(scoresList);
		thisServer->SendGlobalPacket(*msgFromServer);
	}
}

void NetworkedGame::UpdateAsClient(float dt) {
	/*ClientPacket newPacket;

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		//fire button pressed!
		newPacket.buttonstates[0] = 1;
		newPacket.lastID = 0; //You'll need to work this out somehow...
	}
	thisClient->SendPacket(newPacket);*/

	//std::cout << "updating as client\n";

	//GamePacket* msgFromClient = new StringPacket(" Client says hello ! " + std::to_string(666));
	//thisClient->SendPacket(*msgFromClient);

	if (player->win && !hasSentScore) {
		GamePacket* msgFromClient = new StringPacket(/*" Player won with score " + */std::to_string(player->score));
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

		if (source >= 0) {	// if message came from client, record score
			std::cout << "server received score (source " << source << "): " << msg << std::endl;	
			numScores++;
			latestScore = std::stoi(msg);	// can probably remove these and put score saving functionality here
			latestClient = source;
		}
		
		else {	// came from server so save scores
			/*scores = {};
			std::stringstream ss(msg);

			/*for (int i; ss >> i;) {
				scores.push_back(i);
				if (ss.peek() == ',')
					ss.ignore();
			}*/
			
			/*while (ss.good()) {
				std::string substr;
				getline(ss, substr, ',');
				scores.push_back(std::stoi(substr));
			}

			std::cout << "CLIENT SCORE LIST: ";
			for (auto i = scores.begin(); i != scores.end(); ++i) {
				std::cout << std::to_string(*i) + ",";
			}
			std::cout << "\n\n";*/

			std::cout << "CLIENT RECIEVED SCORE LIST: " << msg << "\n\n";	// need to process and save scores
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