#pragma once
#include "Vec2.h"

class Entity
{
public:
	Entity() {}
	Entity(int id, Vec2 pos, float radius) : m_id(id), m_pos(pos), m_radius(radius) {}

	int   getId()     const { return m_id; }
	Vec2  getPos()    const { return m_pos; }
	float getRadius() const { return m_radius; }

	void setId(int id) { m_id = id; }
	void setPos(const Vec2 pos) { m_pos = pos; }
	void setRadius(float radius) { m_radius = radius; }

private:
	int   m_id;
	Vec2  m_pos;
	float m_radius;

};