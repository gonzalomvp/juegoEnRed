#include "Vec2.h"

Vec2::Vec2(iVec2 v) : x(v.x), y(v.y) {}
iVec2::iVec2(Vec2 v) : x(roundf(v.x)), y(roundf(v.y)) {}