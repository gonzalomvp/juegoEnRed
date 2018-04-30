#pragma once
#include "common/core.h"

class Player
{
public:
	Player(const vec2& position) : m_position(position) {}

	vec2 getPosition() const               { return m_position;     }
	vec2 setPosition(const vec2& position) { m_position = position; }
private:
	vec2 m_position;
};