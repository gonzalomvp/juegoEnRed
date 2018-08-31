#pragma once

#include "Buffer.h"
#include "Entity.h"
#include <map>
#include <vector>

enum NetMessageType
{
	NETMSG_UNKNOWN,
	NETMSG_STARTMATCH,
	NETMSG_DISCONNECT,
	NETMSG_PLAYERSPOSITIONS,
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

	int playerId;

	int numPickups;
	std::map<int, Entity> pickups;

	int numPlayers;
	std::map<int, Player> players;
};

struct NetMessagePlayersPositions : public NetMessage
{
	NetMessagePlayersPositions() { Type = NETMSG_PLAYERSPOSITIONS; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	int numPlayers;
	std::map<int, Player> players;
};

struct NetMessageAddRemoveEntities : public NetMessage
{
	NetMessageAddRemoveEntities() { Type = NETMSG_ADDREMOVEENTITIES; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	int numPickupsToAdd;
	std::vector<Entity> pickupsToAdd;

	int numPlayersToAdd;
	std::vector<Entity> playersToAdd;

	int numEntitiesToRemove;
	std::vector<int> entitiesToRemove;
};

struct NetMessageMoveCommand : public NetMessage
{
	NetMessageMoveCommand() { Type = NETMSG_MOVECOMMAND; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	Vec2 mousePos;
};
