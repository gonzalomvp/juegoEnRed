#pragma once

#include "Buffer.h"
#include "Pickup.h"
#include "Player.h"
#include <vector>

enum NetMessageType
{
	NETMSG_UNKNOWN,
	NETMSG_STARTMATCH,
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
	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	NetMessageStartMatch() { Type = NETMSG_STARTMATCH; }
	std::vector<Player> players;
	int numPlayers;

	std::vector<Pickup*> pickups;
	int numPickups;
	int id;
};

struct NetMessageMoveCommand : public NetMessage
{
	virtual void serialize(CBuffer& buffer);
	virtual void deserialize(CBuffer& buffer);

	NetMessageMoveCommand() { Type = NETMSG_MOVECOMMAND; }
	int mouseX;
	int mouseY;
};
