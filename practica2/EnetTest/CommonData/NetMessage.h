#pragma once

#include "Buffer.h"
#include "Player.h"
#include <vector>

enum NetMessageType
{
	NETMSG_UNKNOWN,
	NETMSG_STARTMATCH,
	NETMSG_CHANGEPOS
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
};
