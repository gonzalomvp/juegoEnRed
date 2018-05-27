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
	numPickups = pickups.size();
	buffer.Write(&numPickups, sizeof(numPickups));
	for (size_t i = 0; i < numPickups; i++)
	{
		buffer.Write(pickups[i], sizeof(Pickup));
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
	buffer.Read(&numPickups, sizeof(numPickups));
	for (size_t i = 0; i < numPickups; i++)
	{
		Pickup* pickup = new Pickup(0, Vec2());
		buffer.Read(pickup, sizeof(Pickup));
		pickups.push_back(pickup);
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