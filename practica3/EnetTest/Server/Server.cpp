#include "stdafx.h"
#include "Buffer.h"
#include "Entity.h"
#include "NetMessage.h"
#include "ServerENet.h"

#include <ctime>
#include <map>
#include <Windows.h>

#define SCR_WIDTH     800
#define SCR_HEIGHT    600
#define MIN_SPEED       0.5f
#define MAX_SPEED       2.0f
#define MIN_RADIUS     22
#define MAX_RADIUS    122
#define INIT_PICKUPS   20
#define MAX_PICKUPS    50
#define PICKUPS_TIMER 100
#define PICKUPS_RADIUS  5.0f
#define LATENCY_BASE    0.0f
#define PACKETS_SEC     5.0f

using namespace ENet;

void init();
void update();
void checkCollisions();
Player registerPlayer(int id);

float calculateSpeed(float radius);
bool  checkCircleCircle(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2);
bool  checkAbsorb(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2);

std::map<int, Entity> g_pickups;
std::map<int, Player> g_players;
std::vector<Entity>   g_pickupsToAdd;
std::vector<Entity>   g_playersToAdd;
std::vector<int>      g_entitiesToRemove;

int g_idCounter = 0;
int g_timer = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	// Init world
	init();

	// Init Server
	CServerENet* pServer = new CServerENet();
	if (pServer->Init(1234, 5, 0, 0, LATENCY_BASE, 0, 0))
	{
		clock_t beginTime = clock();
		float accumulatedTime = 0.0f;

		CBuffer buffer(2048);
		std::vector<CPacketENet*> incommingPackets;
		// Keep looping checking incoming messages
		while (true)
		{
			clock_t endTime = clock();
			float deltaTime = static_cast<float>(endTime - beginTime);
			beginTime = endTime;
			accumulatedTime += deltaTime;

			incommingPackets.clear();
			pServer->Service(incommingPackets, deltaTime);

			// Process each received message
			for (size_t i = 0; i < incommingPackets.size(); ++i)
			{
				CPacketENet* packet = incommingPackets[i];
				// Data Message
				if (packet->GetType() == EPacketType::DATA)
				{
					NetMessageType type = *reinterpret_cast<NetMessageType*>(packet->GetData());
					switch (type)
					{
						// Move request message received from a client
						case NETMSG_MOVECOMMAND:
						{
							buffer.Clear();
							buffer.Write(packet->GetData(), packet->GetDataLength());
							NetMessageMoveCommand msgMove;
							msgMove.deserialize(buffer);

							// The ID of the client is the peer pointer
							int playerId = reinterpret_cast<int>(packet->GetPeer());
							if (g_players.count(playerId))
							{
								// Update player position
								Vec2 dir = msgMove.mousePos - g_players[playerId].getPos();
								Vec2 velocity = dir.norm() * calculateSpeed(g_players[playerId].getRadius());
								g_players[playerId].setPos(g_players[playerId].getPos() + velocity);
							}
							break;
						}
					}
				}

				// New connection Message
				else if (packet->GetType() == EPacketType::CONNECT)
				{
					// Creates new player using the peer pointer as ID
					Player playerToAdd = registerPlayer(reinterpret_cast<int>(packet->GetPeer()));
					g_playersToAdd.push_back(Entity(playerToAdd.getId(), playerToAdd.getPos()));

					// Send a world snapshot to the new client using a reliable packet
					NetMessageStartMatch message;
					message.playerId = playerToAdd.getId();
					message.pickups = g_pickups;
					message.players = g_players;
					message.serialize(buffer);
					pServer->SendData(packet->GetPeer(), buffer.GetBytes(), buffer.GetSize(), 0, true);
				}

				// Disconnection Message
				else if (packet->GetType() == EPacketType::DISCONNECT)
				{
					int playerId = reinterpret_cast<int>(packet->GetPeer());
					// Removed the player if exists
					if (g_players.count(playerId))
					{
						g_entitiesToRemove.push_back(playerId);
						g_players.erase(playerId);
					}
				}
			}

			// Create new entities and check collisions
			update();

			if (accumulatedTime >= (1000.0f / PACKETS_SEC))
			{
				printf("SNAPSHOT SENT");
				accumulatedTime = 0.0f;
				//Send new world snapshot to all clients using a NOT reliable packet
				NetMessageWorldSnapshot messageWorldSnapshot;
				messageWorldSnapshot.players = g_players;
				messageWorldSnapshot.serialize(buffer);
				pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 1, false);
			}
			
			//Send entities to Add or Remove to all clients using a reliable packet
			if (g_pickupsToAdd.size() > 0 || g_playersToAdd.size() > 0 || g_entitiesToRemove.size() > 0)
			{
				NetMessageAddRemoveEntities messageAddRemoveEntities;
				messageAddRemoveEntities.pickupsToAdd = g_pickupsToAdd;
				messageAddRemoveEntities.playersToAdd = g_playersToAdd;
				messageAddRemoveEntities.entitiesToRemove = g_entitiesToRemove;
				messageAddRemoveEntities.serialize(buffer);
				pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, true);
				g_pickupsToAdd.clear();
				g_playersToAdd.clear();
				g_entitiesToRemove.clear();
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
	// Create initial pickups
	srand(timeGetTime());
	for (size_t i = 0; i < INIT_PICKUPS; i++)
	{
		Vec2 pos(static_cast<float>(rand() % SCR_WIDTH), static_cast<float>(rand() % SCR_HEIGHT));
		Entity pickup(g_idCounter++, pos);
		g_pickups[pickup.getId()] = pickup;
	}
}

void update()
{
	// Periodically creates new pickups
	++g_timer;
	if (g_timer >= PICKUPS_TIMER)
	{
		g_timer = 0;
		if (g_pickups.size() < MAX_PICKUPS)
		{
			Vec2 pos(static_cast<float>(rand() % SCR_WIDTH), static_cast<float>(rand() % SCR_HEIGHT));
			Entity pickup(g_idCounter++, pos);
			g_pickups[pickup.getId()] = pickup;
			g_pickupsToAdd.push_back(pickup);
		}
	}

	checkCollisions();
}

void checkCollisions() {
	auto itPlayer = g_players.begin();
	while (itPlayer != g_players.end())
	{
		// Check collisions between players
		Player& p1 = itPlayer->second;
		for (auto itPlayerNext = std::next(itPlayer); itPlayerNext != g_players.end(); ++itPlayerNext)
		{
			Player& p2 = itPlayerNext->second;
			if (checkAbsorb(p1.getPos(), p1.getRadius(), p2.getPos(), p2.getRadius()))
			{
				p1.setRadius(static_cast<unsigned short>(round(p1.getRadius() + p2.getRadius() * 0.5f)));
				g_entitiesToRemove.push_back(p2.getId());
				g_players.erase(itPlayerNext);
				break;
			}
			else if (checkAbsorb(p2.getPos(), p2.getRadius(), p1.getPos(), p1.getRadius()))
			{
				p2.setRadius(static_cast<unsigned short>(round(p2.getRadius() + p1.getRadius() * 0.5f)));
				g_entitiesToRemove.push_back(p1.getId());
				itPlayer = g_players.erase(itPlayer);
				p1 = itPlayer->second;
				break;
			}
		}

		// Check collisions with pickups
		for (auto itPickup = g_pickups.begin(); itPickup != g_pickups.end(); ++itPickup)
		{
			Entity& pickup = itPickup->second;
			if (checkAbsorb(p1.getPos(), p1.getRadius(), pickup.getPos(), PICKUPS_RADIUS))
			{
				p1.setRadius(p1.getRadius() + 2);
				g_entitiesToRemove.push_back(pickup.getId());
				g_pickups.erase(itPickup);
				break;
			}
		}
		++itPlayer;
	}
}

bool checkCircleCircle(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
	return pos1.distance(pos2) < radius1 + radius2;
}

bool checkAbsorb(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
	return pos1.distance(pos2) + radius2 <= radius1;
}

Player registerPlayer(int id)
{
	// Create player
	//Vec2 pos(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
	Vec2 pos(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);
	Player player(id, pos, MIN_RADIUS);
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