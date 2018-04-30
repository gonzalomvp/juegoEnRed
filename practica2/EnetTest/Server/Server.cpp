#include "common/stdafx.h"
#include "Buffer.h"
#include "ServerENet.h"
#include "Player.h"
#include <Windows.h>

using namespace ENet;

int _tmain(int argc, _TCHAR* argv[])
{
	// Create player
	Player player(vmake(100.0, 100.0));
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
			//pServer->SendAll(&player, sizeof(player), 0, false);
			pServer->SendAll("prueba\0", 7, 0, false);
            Sleep(100);
        }
    }
    else
    {
        fprintf(stdout, "Server could not be initialized.\n");
    }

    return 0;
}

