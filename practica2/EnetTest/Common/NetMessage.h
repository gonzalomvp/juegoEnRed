#pragma once

#include "Buffer.h"
#include "Entity.h"
#include "Pickup.h"
#include "Player.h"
#include <map>
#include <vector>

enum NetMessageType
{
	NETMSG_UNKNOWN,
	NETMSG_STARTMATCH,
	NETMSG_DISCONNECT,
	NETMSG_PLAYERSPOSITIONS,
	NETMSG_ADDREMOVEPICKUPS,
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

	Entity player;

	int numPickups;
	std::vector<Entity> pickups;
};

struct NetMessageDisconnect : public NetMessage
{
	NetMessageDisconnect() { Type = NETMSG_DISCONNECT; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	int playerId;
};

struct NetMessagePlayersPositions : public NetMessage
{
	NetMessagePlayersPositions() { Type = NETMSG_PLAYERSPOSITIONS; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	int numPlayers;
	std::map<int, Entity> players;
};

struct NetMessageAddRemovePickups : public NetMessage
{
	NetMessageAddRemovePickups() { Type = NETMSG_ADDREMOVEPICKUPS; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	int numPickupsToAdd;
	std::vector<Entity> pickupsToAdd;

	int numPickupsToRemove;
	std::vector<int> pickupsToRemove;
};

struct NetMessageMoveCommand : public NetMessage
{
	NetMessageMoveCommand() { Type = NETMSG_MOVECOMMAND; }

	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	int playerId;
	Vec2 mousePos;
};
