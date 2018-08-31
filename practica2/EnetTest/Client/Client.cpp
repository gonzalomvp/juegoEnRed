#include "stdafx.h"
#include "Buffer.h"
#include "ClientENet.h"
#include "Entity.h"
#include "NetMessage.h"
#include "PacketENet.h"

#include <map>
#include <Windows.h>

using namespace ENet;

std::map<int, Entity> g_pickups;
std::map<int, Player> g_players;

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
	bool isConnected = false;

	CBuffer buffer(2048);
	std::vector<CPacketENet*> incommingPackets;

	// Init client and connect with server
	CClienteENet* pClient = new CClienteENet();
	pClient->Init();
	CPeerENet* pPeer = pClient->Connect(lpCmdLine, 1234, 2);

	// Keep looping checking incoming messages
	while (!SYS_GottaQuit() && pPeer)
	{
		incommingPackets.clear();
		pClient->Service(incommingPackets, 0);
		
		// Process each received message
		for (size_t i = 0; i < incommingPackets.size(); ++i)
		{
			CPacketENet* packet = incommingPackets[i];
			
			if (packet->GetType() == EPacketType::DATA) {
				NetMessageType type = *reinterpret_cast<NetMessageType*>(packet->GetData());
				buffer.Clear();
				switch (type)
				{
					case NETMSG_STARTMATCH:
					{
						NetMessageStartMatch message;
						buffer.Write(packet->GetData(), packet->GetDataLength());
						message.deserialize(buffer);

						g_pickups = message.pickups;
						g_players = message.players;
						player = g_players[message.playerId];

						isConnected = true;
						break;
					}
					case NETMSG_WORLDSNAPSHOT:
					{
						NetMessageWorldSnapshot message;
						buffer.Write(packet->GetData(), packet->GetDataLength());
						message.deserialize(buffer);
						g_pickups = message.pickups;
						g_players = message.players;
						break;
					}
					
					case NETMSG_ADDREMOVEENTITIES:
					{
						NetMessageAddRemoveEntities message;
						buffer.Write(packet->GetData(), packet->GetDataLength());
						message.deserialize(buffer);

						for (size_t i = 0; i < message.numPickupsToAdd; i++)
						{
							g_pickups[message.pickupsToAdd[i].getId()] = message.pickupsToAdd[i];
						}

						for (size_t i = 0; i < message.numPlayersToAdd; i++)
						{
							g_players[message.playersToAdd[i].getId()] = Player(message.playersToAdd[i].getId(), message.playersToAdd[i].getPos(), 22.0f);
						}

						for (size_t i = 0; i < message.numEntitiesToRemove; i++)
						{
							if (message.entitiesToRemove[i] == player.getId())
							{
								pPeer = nullptr;
								break;
							}
							else if(g_pickups.count(message.entitiesToRemove[i]))
							{
								g_pickups.erase(message.entitiesToRemove[i]);
							}
							else if (g_players.count(message.entitiesToRemove[i]))
							{
								g_players.erase(message.entitiesToRemove[i]);
							}
						}

						break;
					}
				}
			}
			else if (packet->GetType() == EPacketType::DISCONNECT)
			{
				pPeer = nullptr;
			}
			
		}

		if (isConnected && pPeer)
		{
			ivec2 sysMousePos = SYS_MousePos();

			if (sysMousePos.x > 0 && sysMousePos.x <= SCR_WIDTH && sysMousePos.y > 0 && sysMousePos.y <= SCR_HEIGHT)
			{
				NetMessageMoveCommand msgMove;
				msgMove.mousePos = Vec2(sysMousePos.x, sysMousePos.y);
				buffer.Clear();
				msgMove.serialize(buffer);

				pClient->SendData(pPeer, buffer.GetBytes(), buffer.GetSize(), 0, true);
			}

			glClear(GL_COLOR_BUFFER_BIT);

			for (auto it = g_pickups.begin(); it != g_pickups.end(); ++it)
			{
				Entity pickup = it->second;
				CORE_RenderCenteredSprite(vmake(pickup.getPos().x, pickup.getPos().y), vmake(10, 10), texture, 1.0f);
			}

			for (auto it = g_players.begin(); it != g_players.end(); ++it)
			{
				Player playerToRender = it->second;
				//CORE_RenderCenteredSprite(vmake(player.getPos().x, player.getPos().y), vmake(player.getRadius() * 2.0f, player.getRadius() * 2.0f), texture, 1.0f);
				if (playerToRender.getId() == player.getId())
				{
					player.setPos(playerToRender.getPos());
					player.setRadius(playerToRender.getRadius());
				}
				else {
					CORE_RenderCenteredRotatedSprite(vmake(playerToRender.getPos().x, playerToRender.getPos().y), vmake(playerToRender.getRadius() * 2.0f, playerToRender.getRadius() * 2.0f), texture, 1.0f, rgbamake(255, 0, 0, 255));
				}
			}
			CORE_RenderCenteredRotatedSprite(vmake(player.getPos().x, player.getPos().y), vmake(player.getRadius() * 2.0f, player.getRadius() * 2.0f), texture, 1.0f, rgbamake(0, 255, 0, 255));
		}


		

		SYS_Show();
		SYS_Pump();
		SYS_Sleep(17);
	}

	if (pPeer)
	{
		pClient->Disconnect(pPeer);
	}

	FONT_End();
	CORE_EndSound();
	return 0;
}