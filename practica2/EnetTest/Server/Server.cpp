#include "common/stdafx.h"
#include "Buffer.h"
#include "ServerENet.h"
#include "Player.h"
#include "NetMessage.h"
#include <Windows.h>

using namespace ENet;

int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<Player> players;

	// Create player
	Player player(200.0, 200.0, 32.0f);
	players.push_back(player);

	CBuffer buffer;
	buffer.Write(&player, sizeof(player));
	buffer.GotoStart();

	CServerENet* pServer = new CServerENet();
	if (pServer->Init(1234, 5))
	{
		while (true)
		{
			std::vector<CPacketENet*> incommingPackets;
			pServer->Service(incommingPackets, 0);
			CBuffer buffer;
			NetMessageStartMatch message;
			message.players = players;
			message.serialize(buffer);
			//pServer->SendAll(&player, sizeof(player), 0, false);
			pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, false);
			Sleep(100);
		}
	}
	else
	{
		fprintf(stdout, "Server could not be initialized.\n");
	}

	return 0;
}
