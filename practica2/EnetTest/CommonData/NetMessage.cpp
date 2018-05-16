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
	buffer.Write(&id, sizeof(id));
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
	buffer.Read(&id, sizeof(id));
	buffer.Read(&numPlayers, sizeof(numPlayers));
	for (size_t i = 0; i < numPlayers; i++)
	{
		Player player;
		buffer.Read(&player, sizeof(Player));
		players.push_back(player);
	}
}

void NetMessageMoveCommand::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);
	buffer.Write(&mouseX, sizeof(mouseX));
	buffer.Write(&mouseY, sizeof(mouseY));
}

void NetMessageMoveCommand::deserialize(CBuffer& buffer)
{
	NetMessage::deserialize(buffer);
	buffer.Read(&mouseX, sizeof(mouseX));
	buffer.Read(&mouseY, sizeof(mouseY));
}