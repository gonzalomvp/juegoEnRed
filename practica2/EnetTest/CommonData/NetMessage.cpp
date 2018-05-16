#include "NetMessage.h"
#include "Player.h"

void NetMessage::serialize(CBuffer& buffer)
{
	buffer.Write(&Type, sizeof(NetMessageType));
}

void NetMessage::deserialize(CBuffer& buffer)
{
	buffer.Read(&Type, sizeof(NetMessageType));
}

void NetMessageStartMatch::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);
	numPlayers = players.size();
	buffer.Write(&numPlayers, sizeof(numPlayers));
	for (size_t i = 0; i < numPlayers; i++)
	{
		buffer.Write(&players[i], sizeof(players[i]));
	}
	
}

void NetMessageStartMatch::deserialize(CBuffer& buffer)
{
	NetMessage::deserialize(buffer);
	buffer.Read(&numPlayers, sizeof(numPlayers));
	for (size_t i = 0; i < numPlayers; i++)
	{
		Player player;
		buffer.Read(&player, sizeof(Player));
		players.push_back(player);
	}
}