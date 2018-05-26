#pragma once
#include "Vec2.h"

class Player
{
public:
	Player() {}
	Player(float posX, float posY, float radius) : m_posX(posX), m_posY(posY), m_radius(radius) {}

	float m_posX;
	float m_posY;
	float m_radius;
	Vec2 pos;
};