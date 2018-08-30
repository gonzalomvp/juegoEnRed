#pragma once
#include "Vec2.h"

class Entity
{
public:
	Entity() {}
	Entity(int id, iVec2 pos) : m_id(id), m_pos(pos) {}

	int   getId()     const { return m_id; }
	iVec2  getPos()    const { return m_pos; }

	void setId(int id) { m_id = id; }
	void setPos(const iVec2 pos) { m_pos = pos; }

private:
	int   m_id;
	iVec2  m_pos;

};

class Player : public Entity
{
public:
	Player() {}
	Player(int id, iVec2 pos, unsigned short radius) : Entity(id, pos), m_radius(radius) {}

	unsigned short getRadius() const { return m_radius; }
	void setRadius(unsigned short radius) { m_radius = radius; }

private:
	unsigned short m_radius;

};