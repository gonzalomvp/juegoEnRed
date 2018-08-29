#include "stdafx.h"
#include "Buffer.h"
#include "ClientENet.h"
#include "Entity.h"
#include "PacketENet.h"
#include "Pickup.h"
#include "Player.h"
#include "NetMessage.h"
#include <Windows.h>
#include <map>

using namespace ENet;

std::map<int, Entity> g_pickups;
std::map<int, Entity> g_players;

int Main(LPSTR lpCmdLine)
{
	CORE_InitSound();
	FONT_Init();
	//
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // Sets up clipping
	glClearColor(0.0f, 0.1f, 0.3f, 0.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, SCR_WIDTH, 0.0, SCR_HEIGHT, 0.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Entity player;
	bool isAlive = true;
	bool isConnected = false;

	GLuint texture = CORE_LoadPNG("data/bubble.png", false);

	CBuffer buffer(2048);

	CClienteENet* pClient = new CClienteENet();
	pClient->Init();

	CPeerENet* pPeer = pClient->Connect(lpCmdLine, 1234, 2);

	std::vector<CPacketENet*> incommingPackets;
	pClient->Service(incommingPackets, 0);
	Sleep(100);
	//pClient->SendData(pPeer, "pepe", 4, 0, false);

	while (pPeer && !SYS_GottaQuit() && isAlive)
	{
		
		std::vector<CPacketENet*>  incommingPackets;
		pClient->Service(incommingPackets, 0);
		
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
						buffer.GotoStart();
						message.deserialize(buffer);

						player = message.player;

						for (size_t i = 0; i < message.numPickups; i++)
						{
							g_pickups[message.pickups[i].getId()] = message.pickups[i];
						}
						isConnected = true;
						break;
					}
					case NETMSG_PLAYERSPOSITIONS:
					{
						NetMessagePlayersPositions message;
						buffer.Write(packet->GetData(), packet->GetDataLength());
						buffer.GotoStart();
						message.deserialize(buffer);
						g_players = message.players;
						break;
					}
					
					case NETMSG_ADDREMOVEPICKUPS:
					{
						NetMessageAddRemovePickups message;
						buffer.Write(packet->GetData(), packet->GetDataLength());
						buffer.GotoStart();
						message.deserialize(buffer);

						for (size_t i = 0; i < message.numPickupsToAdd; i++)
						{
							g_pickups[message.pickupsToAdd[i].getId()] = message.pickupsToAdd[i];
						}

						for (size_t i = 0; i < message.numPickupsToRemove; i++)
						{
							if (message.pickupsToRemove[i] == player.getId())
							{
								isAlive = false;
							}
							else
							{
								g_pickups.erase(message.pickupsToRemove[i]);
							}
						}

						break;
					}
				}
			}
			else if (packet->GetType() == EPacketType::DISCONNECT) {
				isAlive = false;
			}
			
		}

		if (isConnected)
		{
			ivec2 sysMousePos = SYS_MousePos();

			if (sysMousePos.x > 0 && sysMousePos.x <= SCR_WIDTH && sysMousePos.y > 0 && sysMousePos.y <= SCR_HEIGHT)
			{
				NetMessageMoveCommand msgMove;
				msgMove.playerId = player.getId();
				msgMove.mousePos = Vec2(sysMousePos.x, sysMousePos.y);
				buffer.Clear();
				msgMove.serialize(buffer);

				pClient->SendData(pPeer, buffer.GetBytes(), buffer.GetSize(), 0, true);
			}

			glClear(GL_COLOR_BUFFER_BIT);

			for (auto it = g_players.begin(); it != g_players.end(); ++it)
			{
				Entity playerToRender = it->second;
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

			for (auto it = g_pickups.begin(); it != g_pickups.end(); ++it)
			{
				Entity pickup = it->second;
				CORE_RenderCenteredSprite(vmake(pickup.getPos().x, pickup.getPos().y), vmake(10, 10), texture, 1.0f);
			}
			CORE_RenderCenteredRotatedSprite(vmake(player.getPos().x, player.getPos().y), vmake(player.getRadius() * 2.0f, player.getRadius() * 2.0f), texture, 1.0f, rgbamake(0, 255, 0, 255));
		}


		

		SYS_Show();
		SYS_Pump();
		SYS_Sleep(17);
	}

	NetMessageDisconnect msgDisconnect;
	msgDisconnect.playerId = player.getId();
	buffer.Clear();
	msgDisconnect.serialize(buffer);
	//pClient->SendData(pPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);

	if (pPeer)
	{
		pClient->Disconnect(pPeer);
	}

	FONT_End();
	CORE_EndSound();
	return 0;
}