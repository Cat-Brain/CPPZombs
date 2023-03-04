#include "Texture.h"

typedef glm::ivec2 iVec2;

#pragma region iVec2 functions
inline iVec2 RotateLeft(iVec2 a)
{
	return iVec2(-a.y, a.x);
}

iVec2 RotateLeft(iVec2 a, int amount)
{
	int rotation = JMod(amount, 4);
	return iVec2(a.x * int(rotation == 0) - a.y * int(rotation == 1) - a.x * int(rotation = 2) + a.y * int(rotation == 3),
		a.y * int(rotation == 0) + a.x * int(rotation == 1) - a.y * int(rotation = 2) - a.x * int(rotation == 3));
}

inline iVec2 RotateRight(iVec2 a)
{
	return iVec2(a.y, -a.x);
}

iVec2 RotateRight(iVec2 a, int amount)
{
	int rotation = JMod(amount, 4);
	return iVec2(a.x * int(rotation == 0) + a.y * int(rotation == 1) - a.x * int(rotation = 2) - a.y * int(rotation == 3),
		a.y * int(rotation == 0) - a.x * int(rotation == 1) - a.y * int(rotation = 2) + a.x * int(rotation == 3));
}

inline iVec2 RotateRight45(iVec2 a)
{
	return iVec2(int((a.x + a.y) / 1.41f), int((a.y - a.x) / 1.41f));
}

inline iVec2 ClampV(iVec2 a, iVec2 minimum, iVec2 maximum)
{
	return iVec2(Clamp(a.x, minimum.x, maximum.x), Clamp(a.y, minimum.y, maximum.y));
}

inline iVec2 Abs(iVec2 a)
{
	return iVec2(abs(a.x), abs(a.y));
}

inline int Squagnitude(iVec2 a)
{
	return static_cast<int>(max(labs(static_cast<long>(a.x)), labs(static_cast<long>(a.y))));
}

inline int Diagnitude(iVec2 a)
{
	return abs(a.x) + abs(a.y);
}

inline int Squistance(iVec2 a, iVec2 b)
{
	return Squagnitude(a - b);
}

inline int Diagnistance(iVec2 a, iVec2 b)
{
	return Diagnitude(a - b);
}

inline iVec2 Squarmalized(iVec2 a)
{
	return a / max(1, Squagnitude(a));
}
#pragma endregion

iVec2 upI(0, 1), rightI(1, 0), downI(0, -1), leftI(-1, 0), vZeroI(0, 0), vOneI(1, 1);

typedef glm::vec2 Vec2;

#pragma region Vec2 functions
inline Vec2 RotateLeft(Vec2 a)
{
	return Vec2(-a.y, a.x);
}

inline Vec2 RotateLeft(Vec2 a, int amount)
{
	int rotation = JMod(amount, 4);
	return Vec2(a.x * int(rotation == 0) - a.y * int(rotation == 1) - a.x * int(rotation = 2) + a.y * int(rotation == 3),
		a.y * int(rotation == 0) + a.x * int(rotation == 1) - a.y * int(rotation = 2) - a.x * int(rotation == 3));
}

inline Vec2 RotateRight(Vec2 a)
{
	return Vec2(a.y, -a.x);
}

inline Vec2 RotateRight(Vec2 a, int amount)
{
	return RotateLeft(a, -amount);
}

inline Vec2 RotateRight45(Vec2 a)
{
	return Vec2((a.x + a.y) / 1.41f, (a.y - a.x) / 1.41f);
}

inline Vec2 ClampV(Vec2 a, Vec2 minimum, Vec2 maximum)
{
	return Vec2(ClampF(a.x, minimum.x, maximum.x), ClampF(a.y, minimum.y, maximum.y));
}

inline Vec2 Abs(Vec2 a)
{
	return Vec2(abs(a.x), abs(a.y));
}

inline Vec2 Normalized(Vec2 a) // Safe version of glm::normalize
{
	if (a == Vec2(0))
		return Vec2(0);
	return glm::normalize(a);
}

inline iVec2 Rormalized(Vec2 a)
{
	Vec2 normalized = Normalized(a);
	return iVec2(static_cast<int>(roundf(normalized.x)), static_cast<int>(roundf(normalized.y)));
}

inline Vec2 V2fMin(Vec2 a, Vec2 b)
{
	return glm::length2(a) < glm::length2(b) ? a : b;
}

inline float Distance(Vec2 a, Vec2 b)
{
	return glm::length(a - b);
}

inline Vec2 Ceil(Vec2 a)
{
	return Vec2(ceilf(a.x), ceilf(a.y));
}

inline Vec2 CircPoint(float rotation)
{
	return Vec2(cosf(rotation), sinf(rotation));
}

inline Vec2 RandCircPoint()
{
	return CircPoint(RandFloat() * 2 * PI_F);
}
#pragma endregion

Vec2 up(0, 1), right(1, 0), down(0, -1), left(-1, 0), vZero(0), vOne(1);