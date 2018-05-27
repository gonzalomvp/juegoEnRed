#include "stdafx.h"
#include "Buffer.h"
#include "ServerENet.h"
#include "Pickup.h"
#include "Player.h"
#include "NetMessage.h"

#include <ctime>
#include <Windows.h>

#define SCR_WIDTH  640
#define SCR_HEIGHT 480

using namespace ENet;

void init();
void update();
void checkCollisions();

bool checkCircleCircle(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2);


std::vector<Pickup*> g_pickups;
std::vector<Player> players;
int g_idCounter = 0;
int g_timer = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	

	// Create player
	Player player(200.0, 200.0, 16.0f);
	players.push_back(player);

	// Init World
	init();

	CBuffer buffer;
	buffer.Write(&player, sizeof(player));
	buffer.GotoStart();

	CServerENet* pServer = new CServerENet();
	if (pServer->Init(1234, 5))
	{
		while (true)
		{
			update();
			std::vector<CPacketENet*> incommingPackets;
			pServer->Service(incommingPackets, 0);
			CBuffer buffer;
			NetMessageStartMatch message;
			message.players = players;
			message.pickups = g_pickups;
			message.serialize(buffer);
			//pServer->SendAll(&player, sizeof(player), 0, false);
			pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, false);
			pServer->Service(incommingPackets, 0);

			for (size_t i = 0; i < incommingPackets.size(); ++i)
			{
				CPacketENet* packet = incommingPackets[i];
				if (packet->GetType() == EPacketType::DATA) {
					NetMessageType type = *reinterpret_cast<NetMessageType*>(packet->GetData());
					switch (type)
					{
						case NETMSG_MOVECOMMAND:
							buffer.Clear();
							buffer.Write(packet->GetData(), packet->GetDataLength());
							buffer.GotoStart();
							NetMessageMoveCommand msgMove;
							msgMove.deserialize(buffer);
							int dirX = msgMove.mouseX - players[0].m_posX;
							int dirY = msgMove.mouseY - players[0].m_posY;
							float length = sqrt(dirX * dirX + dirY * dirY);
							players[0].m_posX += dirX / length;
							players[0].m_posY += dirY / length;
							break;
					}
				}
			}


			Sleep(17);
		}
	}
	else
	{
		fprintf(stdout, "Server could not be initialized.\n");
	}

	return 0;
}

void init() {
	srand(timeGetTime());
	for (size_t i = 0; i < 10; i++) {
		Vec2 pos(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
		Pickup* pickup = new Pickup(g_idCounter++, pos);
		g_pickups.push_back(pickup);
	}
}

void update() {
	++g_timer;
	if (g_timer >= 100) {
		g_timer = 0;
		Vec2 pos(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
		Pickup* pickup = new Pickup(g_idCounter++, pos);
		g_pickups.push_back(pickup);
	}
	checkCollisions();
}

void checkCollisions() {
	for (auto it = g_pickups.begin(); it != g_pickups.end(); ++it) {
		if (checkCircleCircle(Vec2(players[0].m_posX, players[0].m_posY), players[0].m_radius, (*it)->getPos(), 5)) {
			delete *it;
			g_pickups.erase(it);
			players[0].m_radius = players[0].m_radius + 1;
			break;
		}
	}
}

bool checkCircleCircle(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
	return pos1.distance(pos2) < radius1 + radius2;
}