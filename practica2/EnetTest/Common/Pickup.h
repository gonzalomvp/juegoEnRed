#pragma once

#include "Vec2.h"

class Pickup {
public:
	Pickup() {}
	Pickup(int id, Vec2 pos) : m_id(id), m_pos(pos) {}
	int getId() const { return m_id; }
	Vec2 getPos() const { return m_pos; }

private:
	int  m_id;
	Vec2 m_pos;
};