#include "stdafx.h"
#include "Buffer.h"
#include "ServerENet.h"
#include "Pickup.h"
#include "Player.h"
#include "NetMessage.h"

#include <ctime>
#include <Windows.h>
#include <map>

#define SCR_WIDTH  640
#define SCR_HEIGHT 480

using namespace ENet;

void init();
void update();
void checkCollisions();

bool checkCircleCircle(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2);
bool checkAbsorve(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2);


std::vector<Pickup> g_pickups;
//std::vector<Player> players;
std::map<int, Player> g_players;

std::vector<Pickup> g_pickupsToAdd;
std::vector<int>    g_pickupsToRemove;

int g_idCounter = 0;
int g_timer = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	// Create player
	Player player(1, Vec2(200.0f, 200.0f), 22.0f);
	g_players[1] = player;

	player = Player(2, Vec2(100.0f, 100.0f), 16.0f);
	g_players[2] = player;

	// Init World
	init();

	CBuffer buffer;
	CServerENet* pServer = new CServerENet();
	if (pServer->Init(1234, 5))
	{
		while (true)
		{
			
			std::vector<CPacketENet*> incommingPackets;
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
							Vec2 dir = msgMove.mousePos - g_players[1].getPos();
							g_players[1].setPos(g_players[1].getPos() + dir.norm());
							break;
					}
				}
				else if (packet->GetType() == EPacketType::CONNECT) {
					NetMessageStartMatch message;
					message.player = g_players[1];
					message.pickups = g_pickups;
					message.serialize(buffer);
					pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, false);
				}
			}

			update();

			//Send players
			NetMessagePlayersPositions messagePlayers;
			messagePlayers.players = g_players;
			messagePlayers.serialize(buffer);
			pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, false);

			//Send entities to Add or remove
			NetMessageAddRemovePickups messagePickups;
			messagePickups.pickupsToAdd    = g_pickupsToAdd;
			messagePickups.pickupsToRemove = g_pickupsToRemove;
			messagePickups.serialize(buffer);
			pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, false);
			g_pickupsToAdd.clear();
			g_pickupsToRemove.clear();

			Sleep(17);
		}
	}
	else
	{
		fprintf(stdout, "Server could not be initialized.\n");
	}

	return 0;
}

void init()
{
	srand(timeGetTime());
	for (size_t i = 0; i < 10; i++)
	{
		Vec2 pos(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
		Pickup pickup(g_idCounter++, pos);
		g_pickups.push_back(pickup);
	}
}

void update() {
	++g_timer;
	if (g_timer >= 100)
	{
		g_timer = 0;
		Vec2 pos(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
		Pickup pickup(g_idCounter++, pos);
		g_pickups.push_back(pickup);
		g_pickupsToAdd.push_back(pickup);
	}
	checkCollisions();
}

void checkCollisions() {
	for (auto it = g_players.begin(); std::next(it) != g_players.end(); ++it)
	{
		Player p1 = it->second;
		Player p2 = std::next(it)->second;

		if (checkAbsorve(p1.getPos(), p1.getRadius(), p2.getPos(), p2.getRadius()))
		{
			g_players.erase(std::next(it));
			g_players[p1.getId()].setRadius(p1.getRadius() + p2.getRadius());
			break;
		}
	}

	for (auto it = g_pickups.begin(); it != g_pickups.end(); ++it) 
	{
		if (checkCircleCircle(g_players[1].getPos(), g_players[1].getRadius(), (*it).getPos(), 5))
		{
			g_players[1].setRadius(g_players[1].getRadius() + 1);
			g_pickupsToRemove.push_back(it->getId());
			g_pickups.erase(it);
			break;
		}
	}
}

bool checkCircleCircle(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
	return pos1.distance(pos2) < radius1 + radius2;
}

bool checkAbsorve(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
	return pos1.distance(pos2) + radius2 <= radius1;
}