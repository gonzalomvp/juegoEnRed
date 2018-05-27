#include "stdafx.h"
#include "Buffer.h"
#include "ClientENet.h"
#include "PacketENet.h"
#include "Pickup.h"
#include "Player.h"
#include "NetMessage.h"
#include <Windows.h>

using namespace ENet;

std::vector<Pickup*> g_pickups;

int Main(void)
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
	GLuint texture = CORE_LoadPNG("data/ball.png", false);

	CClienteENet* pClient = new CClienteENet();
	pClient->Init();

	CPeerENet* pPeer = pClient->Connect("127.0.0.1", 1234, 2);

	std::vector<CPacketENet*>  incommingPackets;
	pClient->Service(incommingPackets, 0);
	Sleep(100);
	//pClient->SendData(pPeer, "pepe", 4, 0, false);

	std::vector<Player> players;
	while (!SYS_GottaQuit())
	{
		std::vector<CPacketENet*>  incommingPackets;
		pClient->Service(incommingPackets, 0);
		CBuffer buffer;

		NetMessageStartMatch message;
		message.numPlayers = 0;
		for (size_t i = 0; i < incommingPackets.size(); ++i)
		{
			CPacketENet* packet = incommingPackets[i];
			
			if (packet->GetType() == EPacketType::DATA) {
				NetMessageType type = *reinterpret_cast<NetMessageType*>(packet->GetData());
				switch (type)
				{
					case NETMSG_STARTMATCH:
						buffer.Clear();
						buffer.Write(packet->GetData(), packet->GetDataLength());
						buffer.GotoStart();
						message.deserialize(buffer);
						players = message.players;
						g_pickups = message.pickups;
						break;
				}
			}
			
			
		}

		ivec2 sysMousePos = SYS_MousePos();
		NetMessageMoveCommand msgMove;
		msgMove.mouseX = sysMousePos.x;
		msgMove.mouseY = sysMousePos.y;
		buffer.Clear();
		msgMove.serialize(buffer);

		pClient->SendData(pPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);


		glClear(GL_COLOR_BUFFER_BIT);
	
		for (size_t i = 0; i < players.size(); i++)
		{
			Player player = players[i];
			CORE_RenderCenteredSprite(vmake(player.m_posX, player.m_posY), vmake(player.m_radius * 2.0f, player.m_radius * 2.0f), texture, 1.0f);
		}
		for (size_t i = 0; i < g_pickups.size(); i++)
		{
			Pickup* pickup = g_pickups[i];
			CORE_RenderCenteredSprite(vmake(pickup->getPos().x, pickup->getPos().y), vmake(10, 10), texture, 1.0f);
		}
		SYS_Show();
		SYS_Pump();
		SYS_Sleep(17);
	}

	pClient->Disconnect(pPeer);

	FONT_End();
	CORE_EndSound();
	return 0;
}