#include "NetMessage.h"
#include "Player.h"

void NetMessage::serialize(CBuffer& buffer)
{
	buffer.Clear();
	buffer.Write(&Type, sizeof(NetMessageType));
}

void NetMessage::deserialize(CBuffer& buffer)
{
	buffer.Read(&Type, sizeof(NetMessageType));
}

void NetMessageStartMatch::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);
	buffer.Write(&player, sizeof(player));

	numPickups = pickups.size();
	buffer.Write(&numPickups, sizeof(numPickups));

	for (size_t i = 0; i < numPickups; i++)
	{
		buffer.Write(&pickups[i], sizeof(pickups[i]));
	}
}

void NetMessageStartMatch::deserialize(CBuffer& buffer)
{
	NetMessage::deserialize(buffer);

	buffer.Read(&player, sizeof(player));

	buffer.Read(&numPickups, sizeof(numPickups));
	for (size_t i = 0; i < numPickups; i++)
	{
		Pickup pickup;
		buffer.Read(&pickup, sizeof(pickup));
		pickups.push_back(pickup);
	}
}

void NetMessagePlayersPositions::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);
	numPlayers = players.size();
	buffer.Write(&numPlayers, sizeof(numPlayers));

	for (auto it = players.begin(); it != players.end(); ++it)
	{
		buffer.Write(&(it->second), sizeof(it->second));
	}
}

void NetMessagePlayersPositions::deserialize(CBuffer& buffer)
{
	NetMessage::deserialize(buffer);
	buffer.Read(&numPlayers, sizeof(numPlayers));

	for (size_t i = 0; i < numPlayers; i++)
	{
		Player player;
		buffer.Read(&player, sizeof(player));
		players[player.getId()] = player;
	}
}

void NetMessageAddRemovePickups::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);
	numPickupsToAdd = pickupsToAdd.size();
	buffer.Write(&numPickupsToAdd, sizeof(numPickupsToAdd));

	for (size_t i = 0; i < numPickupsToAdd; i++)
	{
		buffer.Write(&pickupsToAdd[i], sizeof(pickupsToAdd[i]));
	}

	numPickupsToRemove = pickupsToRemove.size();
	buffer.Write(&numPickupsToRemove, sizeof(numPickupsToRemove));

	for (size_t i = 0; i < numPickupsToRemove; i++)
	{
		buffer.Write(&pickupsToRemove[i], sizeof(pickupsToRemove[i]));
	}
}

void NetMessageAddRemovePickups::deserialize(CBuffer& buffer)
{
	NetMessage::deserialize(buffer);
	buffer.Read(&numPickupsToAdd, sizeof(numPickupsToAdd));
	for (size_t i = 0; i < numPickupsToAdd; i++)
	{
		Pickup pickup;
		buffer.Read(&pickup, sizeof(pickup));
		pickupsToAdd.push_back(pickup);
	}

	buffer.Read(&numPickupsToRemove, sizeof(numPickupsToRemove));
	for (size_t i = 0; i < numPickupsToRemove; i++)
	{
		int pickupId;
		buffer.Read(&pickupId, sizeof(pickupId));
		pickupsToRemove.push_back(pickupId);
	}
}

void NetMessageMoveCommand::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);
	buffer.Write(&mousePos, sizeof(mousePos));
}

void NetMessageMoveCommand::deserialize(CBuffer& buffer)
{
	NetMessage::deserialize(buffer);
	buffer.Read(&mousePos, sizeof(mousePos));
}