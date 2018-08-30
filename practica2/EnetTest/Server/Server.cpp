#include "stdafx.h"
#include "Buffer.h"
#include "Entity.h"
#include "ServerENet.h"
#include "Pickup.h"
#include "NetMessage.h"

#include <ctime>
#include <Windows.h>
#include <map>

#define SCR_WIDTH  640
#define SCR_HEIGHT 480

#define MIN_SPEED       0.5f
#define MAX_SPEED       2.0f
#define MIN_RADIUS     22
#define MAX_RADIUS    122
#define INIT_PICKUPS   20
#define MAX_PICKUPS    50
#define PICKUPS_TIMER 100

using namespace ENet;

void init();
void update();
void checkCollisions();
Player registerPlayer(int id);

float calculateSpeed(float radius);
bool  checkCircleCircle(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2);
bool  checkAbsorve(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2);


std::vector<Entity> g_pickups;
//std::vector<Player> players;
std::map<int, Player> g_players;
//std::map<CPeerENet*, int> g_peers;

std::vector<Entity> g_pickupsToAdd;
std::vector<int>    g_pickupsToRemove;

int g_idCounter = 0;
int g_timer = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	//// Create player
	//Player player(1, Vec2(200.0f, 200.0f), 22.0f);
	//g_players[1] = player;

	//player = Player(2, Vec2(100.0f, 100.0f), 16.0f);
	//g_players[2] = player;

	// Init World
	init();

	CBuffer buffer(2048);
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
						{
							buffer.Clear();
							buffer.Write(packet->GetData(), packet->GetDataLength());
							buffer.GotoStart();
							NetMessageMoveCommand msgMove;
							msgMove.deserialize(buffer);
							if (g_players.count(msgMove.playerId))
							{
								Vec2 dir = msgMove.mousePos - g_players[msgMove.playerId].getPos();
								Vec2 velocity = dir.norm() * calculateSpeed(g_players[msgMove.playerId].getRadius());
								g_players[msgMove.playerId].setPos(Vec2(g_players[msgMove.playerId].getPos()) + velocity);
							}
							
							break;
						}
							
						case NETMSG_DISCONNECT:
						{
							buffer.Clear();
							buffer.Write(packet->GetData(), packet->GetDataLength());
							buffer.GotoStart();
							NetMessageDisconnect msgDisconnect;
							msgDisconnect.deserialize(buffer);
							if (g_players.count(msgDisconnect.playerId))
							{
								g_players.erase(msgDisconnect.playerId);
							}
							break;
						}
					}
				}
				else if (packet->GetType() == EPacketType::CONNECT) {
					NetMessageStartMatch message;
					message.player = registerPlayer(reinterpret_cast<int>(packet->GetPeer()));
					message.pickups = g_pickups;
					message.serialize(buffer);
					pServer->SendData(packet->GetPeer(), buffer.GetBytes(), buffer.GetSize(), 0, true);
				}
				else if (packet->GetType() == EPacketType::DISCONNECT) {
					if (g_players.count(reinterpret_cast<int>(packet->GetPeer())))
					{
						g_players.erase(reinterpret_cast<int>(packet->GetPeer()));
					}
					//pServer->Disconnect(packet->GetPeer());
				}
			}

			update();

			//Send players
			NetMessagePlayersPositions messagePlayers;
			messagePlayers.players = g_players;
			messagePlayers.serialize(buffer);
			pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 1, false);

			//Send entities to Add or remove
			if (g_pickupsToAdd.size() > 0 || g_pickupsToRemove.size() > 0)
			{
				NetMessageAddRemovePickups messagePickups;
				messagePickups.pickupsToAdd = g_pickupsToAdd;
				messagePickups.pickupsToRemove = g_pickupsToRemove;
				messagePickups.serialize(buffer);
				pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, true);
				g_pickupsToAdd.clear();
				g_pickupsToRemove.clear();
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

void init()
{
	srand(timeGetTime());
	for (size_t i = 0; i < INIT_PICKUPS; i++)
	{
		Vec2 pos(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
		Entity pickup(g_idCounter++, pos);
		g_pickups.push_back(pickup);
	}
}

void update() {
	++g_timer;
	if (g_timer >= PICKUPS_TIMER)
	{
		g_timer = 0;
		if (g_pickups.size() < MAX_PICKUPS)
		{
			Vec2 pos(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
			Entity pickup(g_idCounter++, pos);
			g_pickups.push_back(pickup);
			g_pickupsToAdd.push_back(pickup);
		}
	}
	checkCollisions();
}

void checkCollisions() {
	auto itPlayer = g_players.begin();
	while (itPlayer != g_players.end())
	{
		Player& p1 = itPlayer->second;
		for (auto itPlayerNext = std::next(itPlayer); itPlayerNext != g_players.end(); ++itPlayerNext)
		{
			Player& p2 = itPlayerNext->second;
			if (checkAbsorve(p1.getPos(), p1.getRadius(), p2.getPos(), p2.getRadius()))
			{
				p1.setRadius(p1.getRadius() + p2.getRadius() * 0.5f);
				g_pickupsToRemove.push_back(p2.getId());
				g_players.erase(itPlayerNext);
				break;
			}
			else if (checkAbsorve(p2.getPos(), p2.getRadius(), p1.getPos(), p1.getRadius()))
			{
				p2.setRadius(p2.getRadius() + p1.getRadius() * 0.5f);
				g_pickupsToRemove.push_back(p1.getId());
				itPlayer = g_players.erase(itPlayer);
				p1 = itPlayer->second;
				break;
			}
		}

		for (auto itPickup = g_pickups.begin(); itPickup != g_pickups.end(); ++itPickup)
		{
			if (checkAbsorve(p1.getPos(), p1.getRadius(), (*itPickup).getPos(), 5.0f))
			{
				p1.setRadius(p1.getRadius() + 2);
				g_pickupsToRemove.push_back(itPickup->getId());
				g_pickups.erase(itPickup);
				break;
			}
		}
		++itPlayer;
	}

	//for (auto it = g_pickups.begin(); it != g_pickups.end(); ++it) 
	//{
	//	if (checkCircleCircle(g_players[1].getPos(), g_players[1].getRadius(), (*it).getPos(), 5))
	//	{
	//		g_players[1].setRadius(g_players[1].getRadius() + 1);
	//		g_pickupsToRemove.push_back(it->getId());
	//		g_pickups.erase(it);
	//		break;
	//	}
	//}
}

bool checkCircleCircle(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
	return pos1.distance(pos2) < radius1 + radius2;
}

bool checkAbsorve(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
	return pos1.distance(pos2) + radius2 <= radius1;
}

Player registerPlayer(int id)
{
	// Create player

	Player player(id, Vec2(200.0f, 200.0f), 22.0f);
	g_players[id] = player;
	return player;
}

float calculateSpeed(float radius)
{
	//clamp and normalize radius
	if (radius < MIN_RADIUS) radius = MIN_RADIUS;
	if (radius > MAX_RADIUS) radius = MAX_RADIUS;
	float normalizedRadius = (radius - MIN_RADIUS) / (MAX_RADIUS - MIN_RADIUS);

	// lerp speed based on normalized radius
	float speed = normalizedRadius * (MIN_SPEED - MAX_SPEED) + MAX_SPEED;
	return speed;
}