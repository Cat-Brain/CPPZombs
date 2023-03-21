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

inline Vec2 RotateBy(Vec2 a, float rotation)
{
	float sinTheta = sinf(rotation), cosTheta = cosf(rotation);
	return Vec2(cosTheta * a.x - sinTheta * a.y, sinTheta * a.x + cosTheta * a.y);
}

inline Vec2 RotateTowardsNorm(Vec2 currentDir, Vec2 desiredDir, float moveAmount)
{
	float currentRotation = atan2f(currentDir.y, currentDir.x);

	currentRotation -= (roundf(ModF(atan2f(desiredDir.y, desiredDir.x) - currentRotation, PI_F * 2) / (PI_F * 2)) * 2 - 1) * moveAmount;

	return Vec2(cosf(currentRotation), sinf(currentRotation));
}

inline Vec2 RotateTowards(Vec2 currentDir, Vec2 desiredDir, float moveAmount)
{
	return RotateTowardsNorm(currentDir, desiredDir, moveAmount) * glm::length(currentDir);
}

inline Vec2 FromTo(Vec2 from, Vec2 to, float travelDistance, float desiredDistance = 0) // Doesn't overshoot.
{
	float currentDistance = glm::distance(from, to) + 0.0001f; // Add an epsilon to avoid divide by 0 errors.
	return travelDistance + desiredDistance < currentDistance ? from + (to - from) * (travelDistance / currentDistance) :
		to + (from - to) * (desiredDistance / currentDistance);
}

inline Vec2 TryAdd(Vec2 original, Vec2 additional, float maxMagnitude)
{
	original += additional;
	float sqrMagnitude = glm::length2(original);
	if (sqrMagnitude > maxMagnitude * maxMagnitude)
		return original / sqrtf(sqrMagnitude) * maxMagnitude;
	return original;
}

inline Vec2 TryAdd2(Vec2 original, Vec2 additional, float maxMagnitude)
{
	float oSqrMagnitude = glm::length2(original);
	original += additional;
	float sqrMagnitude = glm::length2(original);
	if (sqrMagnitude > maxMagnitude * maxMagnitude)
	{
		return original / sqrtf(sqrMagnitude) * max(maxMagnitude, sqrtf(oSqrMagnitude));
	}
	return original;
}
#pragma endregion

Vec2 up(0, 1), right(1, 0), down(0, -1), left(-1, 0), vZero(0), vOne(1);