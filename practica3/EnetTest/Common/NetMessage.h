#pragma once

#include "Buffer.h"
#include "Entity.h"
#include <map>
#include <vector>

enum NetMessageType
{
	NETMSG_UNKNOWN,
	NETMSG_STARTMATCH,
	NETMSG_WORLDSNAPSHOT,
	NETMSG_ADDREMOVEENTITIES,
	NETMSG_MOVECOMMAND
};

struct NetMessage
{
	NetMessageType Type;
	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);
};

struct NetMessageStartMatch : public NetMessage
{
	NetMessageStartMatch() { Type = NETMSG_STARTMATCH; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	// ID of the new connected client
	int playerId;

	// Current state of pickups in the world
	size_t numPickups;
	std::map<int, Entity> pickups;

	// Current state of players in the world
	size_t numPlayers;
	std::map<int, Player> players;
};

struct NetMessageWorldSnapshot : public NetMessage
{
	NetMessageWorldSnapshot() { Type = NETMSG_WORLDSNAPSHOT; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	// Current state of pickups in the world
	size_t numPickups;
	std::map<int, Entity> pickups;

	// Current state of players in the world
	size_t numPlayers;
	std::map<int, Player> players;
};

struct NetMessageAddRemoveEntities : public NetMessage
{
	NetMessageAddRemoveEntities() { Type = NETMSG_ADDREMOVEENTITIES; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	// Pickups to add
	size_t numPickupsToAdd;
	std::vector<Entity> pickupsToAdd;

	// Players to add
	size_t numPlayersToAdd;
	std::vector<Entity> playersToAdd;

	// Entities to remove
	size_t numEntitiesToRemove;
	std::vector<int> entitiesToRemove;
};

struct NetMessageMoveCommand : public NetMessage
{
	NetMessageMoveCommand() { Type = NETMSG_MOVECOMMAND; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	// Mouse position
	Vec2 mousePos;
};
