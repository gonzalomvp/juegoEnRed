#include "NetMessage.h"

void NetMessage::serialize(CBuffer& buffer)
{
	buffer.Clear();
	buffer.Write(&Type, sizeof(NetMessageType));
}

void NetMessage::deserialize(CBuffer& buffer)
{
	buffer.GotoStart();
	buffer.Read(&Type, sizeof(NetMessageType));
}

void NetMessageStartMatch::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);
	buffer.Write(&playerId, sizeof(playerId));

	numPickups = pickups.size();
	buffer.Write(&numPickups, sizeof(numPickups));
	for (auto it = pickups.begin(); it != pickups.end(); ++it)
	{
		buffer.Write(&(it->second), sizeof(it->second));
	}

	numPlayers = players.size();
	buffer.Write(&numPlayers, sizeof(numPlayers));
	for (auto it = players.begin(); it != players.end(); ++it)
	{
		buffer.Write(&(it->second), sizeof(it->second));
	}
}

void NetMessageStartMatch::deserialize(CBuffer& buffer)
{
	NetMessage::deserialize(buffer);

	buffer.Read(&playerId, sizeof(playerId));

	buffer.Read(&numPickups, sizeof(numPickups));
	for (size_t i = 0; i < numPickups; i++)
	{
		Entity pickup;
		buffer.Read(&pickup, sizeof(pickup));
		pickups[pickup.getId()] = pickup;
	}

	buffer.Read(&numPlayers, sizeof(numPlayers));
	for (size_t i = 0; i < numPlayers; i++)
	{
		Player player;
		buffer.Read(&player, sizeof(player));
		players[player.getId()] = player;
	}
}

void NetMessageWorldSnapshot::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);

	numPlayers = players.size();
	buffer.Write(&numPlayers, sizeof(numPlayers));
	for (auto it = players.begin(); it != players.end(); ++it)
	{
		buffer.Write(&(it->second), sizeof(it->second));
	}
}

void NetMessageWorldSnapshot::deserialize(CBuffer& buffer)
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

void NetMessageAddRemoveEntities::serialize(CBuffer& buffer)
{
	NetMessage::serialize(buffer);

	numPickupsToAdd = pickupsToAdd.size();
	buffer.Write(&numPickupsToAdd, sizeof(numPickupsToAdd));
	for (size_t i = 0; i < numPickupsToAdd; i++)
	{
		buffer.Write(&pickupsToAdd[i], sizeof(pickupsToAdd[i]));
	}

	numPlayersToAdd = playersToAdd.size();
	buffer.Write(&numPlayersToAdd, sizeof(numPlayersToAdd));
	for (size_t i = 0; i < numPlayersToAdd; i++)
	{
		buffer.Write(&playersToAdd[i], sizeof(playersToAdd[i]));
	}

	numEntitiesToRemove = entitiesToRemove.size();
	buffer.Write(&numEntitiesToRemove, sizeof(numEntitiesToRemove));
	for (size_t i = 0; i < numEntitiesToRemove; i++)
	{
		buffer.Write(&entitiesToRemove[i], sizeof(entitiesToRemove[i]));
	}
}

void NetMessageAddRemoveEntities::deserialize(CBuffer& buffer)
{
	NetMessage::deserialize(buffer);

	buffer.Read(&numPickupsToAdd, sizeof(numPickupsToAdd));
	for (size_t i = 0; i < numPickupsToAdd; i++)
	{
		Entity pickup;
		buffer.Read(&pickup, sizeof(pickup));
		pickupsToAdd.push_back(pickup);
	}

	buffer.Read(&numPlayersToAdd, sizeof(numPlayersToAdd));
	for (size_t i = 0; i < numPlayersToAdd; i++)
	{
		Entity player;
		buffer.Read(&player, sizeof(player));
		playersToAdd.push_back(player);
	}

	buffer.Read(&numEntitiesToRemove, sizeof(numEntitiesToRemove));
	for (size_t i = 0; i < numEntitiesToRemove; i++)
	{
		int id;
		buffer.Read(&id, sizeof(id));
		entitiesToRemove.push_back(id);
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