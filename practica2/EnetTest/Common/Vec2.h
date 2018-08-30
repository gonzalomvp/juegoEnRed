#pragma once

#include <cmath>

struct Vec2;
struct iVec2;

struct Vec2 {
	float x;
	float y;

	Vec2(float x = 0, float y = 0) : x(x), y(y) {}
	Vec2(iVec2 v);

	Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
	Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
	Vec2 operator*(const Vec2& other) const { return Vec2(x * other.x, y * other.y); }
	Vec2 operator/(const Vec2& other) const { return Vec2(x / other.x, y / other.y); }
	Vec2 operator+(float scalar) const { return Vec2(x + scalar, y + scalar); }
	Vec2 operator-(float scalar) const { return Vec2(x - scalar, y - scalar); }
	Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
	Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }
	Vec2 abs() const { return Vec2(::abs(x), ::abs(y)); }
	float dot(const Vec2& other) const { return x*other.x + y*other.y; }
	float sqlength() const { return x*x + y*y; }
	float length() const { return sqrt(sqlength()); }
	Vec2 norm() const { float l = length(); return (l != 0) ? *this / l : *this; }
	float angle(const Vec2& other) const { float a = atan2(y - other.y, other.x - x) * 57.2958f; return (a >= 0) ? a : 360 + a; }
	float distance(const Vec2& other) const { return (other - *this).length(); }
};

struct iVec2 {
	short x;
	short y;

	iVec2(short x = 0, short y = 0) : x(x), y(y) {}
	iVec2(Vec2 v);
};
