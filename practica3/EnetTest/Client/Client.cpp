#include "stdafx.h"
#include "Buffer.h"
#include "ClientENet.h"
#include "Entity.h"
#include "NetMessage.h"
#include "PacketENet.h"

#include <ctime>
#include <map>
#include <Windows.h>

#define MIN_SPEED       0.5f
#define MAX_SPEED       2.0f
#define MIN_RADIUS     22
#define MAX_RADIUS    122
#define INIT_PICKUPS   20
#define MAX_PICKUPS    50
#define PICKUPS_TIMER 100
#define PICKUPS_RADIUS  5.0f
#define PACKETS_DELAY   0.0f
#define LOCAL_SIM       1
#define DEAD_RECKONING  1

using namespace ENet;

float calculateSpeed(float radius);

std::map<int, Entity> g_pickups;
std::map<int, Player> g_players;
std::map<int, Player> g_playersInServer;

int Main(LPSTR lpCmdLine)
{
	// Init engine
	CORE_InitSound();
	FONT_Init();
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // Sets up clipping
	glClearColor(0.0f, 0.1f, 0.3f, 0.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, SCR_WIDTH, 0.0, SCR_HEIGHT, 0.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLuint texture = CORE_LoadPNG("data/bubble.png", false);
	
	// Init client variables
	Player player;
	bool isAlive = true;
	bool isConnected = false;
	clock_t beginTime = clock();
	float accumulatedTime = 0.0f;

	CBuffer buffer(2048);
	std::vector<CPacketENet*> incommingPackets;

	// Init client and connect with server
	CClienteENet* pClient = new CClienteENet();
	pClient->Init();
	CPeerENet* pPeer = pClient->Connect(lpCmdLine, 1234, 2);

	// Keep looping checking incoming messages
	while (!SYS_GottaQuit() && pPeer && isAlive)
	{
		accumulatedTime += static_cast<float>(clock() - beginTime);

		incommingPackets.clear();
		pClient->Service(incommingPackets, 0);
		
		// Process each received message
		for (size_t i = 0; i < incommingPackets.size(); ++i)
		{
			CPacketENet* packet = incommingPackets[i];
			
			// Data Message
			if (packet->GetType() == EPacketType::DATA)
			{
				NetMessageType type = *reinterpret_cast<NetMessageType*>(packet->GetData());
				buffer.Clear();
				switch (type)
				{
					// Initial snapshot including client ID from the server
					case NETMSG_STARTMATCH:
					{
						NetMessageStartMatch message;
						buffer.Write(packet->GetData(), packet->GetDataLength());
						message.deserialize(buffer);

						// Init entities and self player reference
						g_pickups = message.pickups;
						g_players = message.players;
						g_playersInServer = message.players;
						player = g_players[message.playerId];

						isConnected = true;
						break;
					}

					// World snapshot
					case NETMSG_WORLDSNAPSHOT:
					{
						NetMessageWorldSnapshot message;
						buffer.Write(packet->GetData(), packet->GetDataLength());
						message.deserialize(buffer);

						// Update world entities
						g_playersInServer = message.players;
						/*if (isConnected && g_players.count(player.getId()))
						{
							player = g_players[player.getId()];
						}*/
						break;
					}
					
					// Entities update
					case NETMSG_ADDREMOVEENTITIES:
					{
						NetMessageAddRemoveEntities message;
						buffer.Write(packet->GetData(), packet->GetDataLength());
						message.deserialize(buffer);

						// Add new pickups to the world
						for (size_t i = 0; i < message.numPickupsToAdd; i++)
						{
							g_pickups[message.pickupsToAdd[i].getId()] = message.pickupsToAdd[i];
						}

						// Add new players to the world
						for (size_t i = 0; i < message.numPlayersToAdd; i++)
						{
							g_players[message.playersToAdd[i].getId()] = Player(message.playersToAdd[i].getId(), message.playersToAdd[i].getPos(), MIN_RADIUS);
						}

						// Remove entities
						for (size_t i = 0; i < message.numEntitiesToRemove; i++)
						{
							// If the entity to remove is the client player finish game
							if (message.entitiesToRemove[i] == player.getId())
							{
								isAlive = false;
								break;
							}
							// Check if the entity to remove is a pickup
							else if(g_pickups.count(message.entitiesToRemove[i]))
							{
								g_pickups.erase(message.entitiesToRemove[i]);
							}
							// Check if the entity to remove is a player
							else if (g_players.count(message.entitiesToRemove[i]))
							{
								g_players.erase(message.entitiesToRemove[i]);
								g_playersInServer.erase(message.entitiesToRemove[i]);
							}
						}
						break;
					}
				}
			}

			// Disconnection Message
			else if (packet->GetType() == EPacketType::DISCONNECT)
			{
				isAlive = false;
			}
			
		}

		if (isConnected && isAlive)
		{
			// Send the mouse position to the server using a reliable packet
			ivec2 sysMousePos = SYS_MousePos();
			Vec2 mousePosition = Vec2(static_cast<float>(sysMousePos.x), static_cast<float>(sysMousePos.y));
			float deltaPosition = (player.getPos() - mousePosition).sqlength();

			if (deltaPosition > 4.0f && sysMousePos.x > 0 && sysMousePos.x <= SCR_WIDTH && sysMousePos.y > 0 && sysMousePos.y <= SCR_HEIGHT)
			{
				if (accumulatedTime >= PACKETS_DELAY)
				{
					beginTime = clock();
					accumulatedTime = 0.0f;

					NetMessageMoveCommand msgMove;
					msgMove.mousePos = mousePosition;
					msgMove.serialize(buffer);
					pClient->SendData(pPeer, buffer.GetBytes(), buffer.GetSize(), 0, true);
				}

				// Simulate local movement
				if (LOCAL_SIM)
				{
					Vec2 dir = mousePosition - player.getPos();
					Vec2 velocity = dir.norm() * calculateSpeed(player.getRadius());
					player.setPos(player.getPos() + velocity);
					g_playersInServer[player.getId()].setPos(player.getPos());
				}
			}

			// Dead Reckoning
			if (DEAD_RECKONING)
			{
				for (auto it = g_playersInServer.begin(); it != g_playersInServer.end(); ++it)
				{
					Player playerInServer = it->second;
					if (g_players.count(playerInServer.getId()))
					{
						g_players[playerInServer.getId()].setRadius(playerInServer.getRadius());
						Vec2 localPos = g_players[playerInServer.getId()].getPos();
						Vec2 serverPos = playerInServer.getPos();
						Vec2 dir = serverPos - localPos;
						deltaPosition = dir.sqlength();
						float speed = calculateSpeed(playerInServer.getRadius());
						if (deltaPosition <= speed)
						{
							g_players[playerInServer.getId()].setPos(playerInServer.getPos());
						}
						else
						{
							Vec2 velocity = dir.norm() * speed;
							g_players[playerInServer.getId()].setPos(localPos + velocity);
						}
					}
					else
					{
						g_players[playerInServer.getId()] = playerInServer;
					}
				}
			}
			else
			{
				g_players = g_playersInServer;
			}
			player = g_players[player.getId()];

			// Paint the entities in screen
			glClear(GL_COLOR_BUFFER_BIT);

			// Paint pickups
			for (auto it = g_pickups.begin(); it != g_pickups.end(); ++it)
			{
				Entity pickup = it->second;
				CORE_RenderCenteredSprite(vmake(pickup.getPos().x, pickup.getPos().y), vmake(PICKUPS_RADIUS * 2.0f, PICKUPS_RADIUS * 2.0f), texture, 1.0f);
			}

			// Paint players except client player
			for (auto it = g_players.begin(); it != g_players.end(); ++it)
			{
				Player playerToRender = it->second;
				if (playerToRender.getId() != player.getId())
				{
					CORE_RenderCenteredRotatedSprite(vmake(playerToRender.getPos().x, playerToRender.getPos().y), vmake(playerToRender.getRadius() * 2.0f, playerToRender.getRadius() * 2.0f), 0.0f, texture, rgbamake(255, 0, 0, 255));
				}
			}

			// Paint client player
			CORE_RenderCenteredRotatedSprite(vmake(player.getPos().x, player.getPos().y), vmake(player.getRadius() * 2.0f, player.getRadius() * 2.0f), 0.0f, texture, rgbamake(0, 255, 0, 255));
		}

		SYS_Show();
		SYS_Pump();
		SYS_Sleep(17);
	}

	// Send disconnection message
	if (pPeer)
	{
		pClient->Disconnect(pPeer);
	}

	FONT_End();
	CORE_EndSound();
	return 0;
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