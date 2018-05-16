#include "common/stdafx.h"
#include "Buffer.h"
#include "ClientENet.h"
#include "PacketENet.h"
#include "Player.h"
#include "NetMessage.h"
#include <Windows.h>

using namespace ENet;

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
	pClient->SendData(pPeer, "pepe", 4, 0, false);
	while (true)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		
		std::vector<CPacketENet*>  incommingPackets;
		pClient->Service(incommingPackets, 0);

		for (size_t i = 0; i < incommingPackets.size(); ++i)
		{
			CPacketENet* packet = incommingPackets[i];
			CBuffer buffer;
			NetMessageType type = *reinterpret_cast<NetMessageType*>(packet->GetData());
			switch (type)
			{
				case NETMSG_STARTMATCH:
					buffer.Write(packet->GetData(), packet->GetDataLength());
					buffer.GotoStart();
					NetMessageStartMatch message;
					message.deserialize(buffer);
					int num = message.numPlayers;
					
					for (size_t i = 0; i < message.numPlayers; i++)
					{
						Player player = message.players[i];
						CORE_RenderCenteredSprite(vmake(player.m_posX, player.m_posY), vmake(player.m_radius, player.m_radius), texture, 1.0f);
					}
					break;
			}
			
		}

		Sleep(100);

		SYS_Show();
		SYS_Pump();
		SYS_Sleep(17);
	}

	pClient->Disconnect(pPeer);

	FONT_End();
	CORE_EndSound();
	return 0;
}