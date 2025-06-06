#include "Texture.h"

typedef glm::ivec2 iVec2;
typedef glm::vec2 Vec2;
typedef glm::ivec3 iVec3;
typedef glm::vec3 Vec3;

iVec2 northI2(0, 1), eastI2(1, 0), southI2(0, -1), westI2(-1, 0), vZeroI2(0), vOneI2(1);
Vec2 north2(0, 1), east2(1, 0), south2(0, -1), west2(-1, 0), vZero2(0), vOne2(1);
iVec3 northI(0, 1, 0), eastI(1, 0, 0), southI(0, -1, 0), westI(-1, 0, 0), upI(0, 0, 1), downI(0, 0, -1), vZeroI(0), vOneI(1);
Vec3 north(0, 1, 0), east(1, 0, 0), south(0, -1, 0), west(-1, 0, 0), up(0, 0, 1), down(0, 0, -1), vZero(0), vOne(1);

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

#pragma region Vec2 functions
inline iVec2 ToIV2(Vec2 a)
{
	return iVec2(floorf(a.x), floorf(a.y));
}

inline Vec2 RotateLeft2(Vec2 a)
{
	return Vec2(-a.y, a.x);
}

inline Vec2 RotateLeft2(Vec2 a, int amount)
{
	int rotation = JMod(amount, 4);
	return Vec2(a.x * int(rotation == 0) - a.y * int(rotation == 1) - a.x * int(rotation = 2) + a.y * int(rotation == 3),
		a.y * int(rotation == 0) + a.x * int(rotation == 1) - a.y * int(rotation = 2) - a.x * int(rotation == 3));
}

inline Vec2 RotateRight2(Vec2 a)
{
	return Vec2(a.y, -a.x);
}

inline Vec2 RotateRight2(Vec2 a, int amount)
{
	return RotateLeft(a, -amount);
}

inline Vec2 RotateRight452(Vec2 a)
{
	return Vec2((a.x + a.y) / 1.41f, (a.y - a.x) / 1.41f);
}

inline Vec2 Normalized2(Vec2 a) // Safe version of glm::normalize
{
	if (a == Vec2(0))
		return Vec2(0);
	return glm::normalize(a);
}

inline iVec2 Rormalized2(Vec2 a)
{
	Vec2 normalized = Normalized2(a);
	return iVec2(static_cast<int>(roundf(normalized.x)), static_cast<int>(roundf(normalized.y)));
}

inline Vec2 V2Min(Vec2 a, Vec2 b)
{
	return glm::length2(a) < glm::length2(b) ? a : b;
}

inline Vec2 V2Max(Vec2 a, Vec2 b)
{
	return glm::length2(a) > glm::length2(b) ? a : b;
}

inline Vec2 Ceil2(Vec2 a)
{
	return Vec2(ceilf(a.x), ceilf(a.y));
}

inline Vec2 CircPoint2(float rotation)
{
	return Vec2(cosf(rotation), sinf(rotation));
}

inline Vec2 RandCircPoint2()
{
	return CircPoint2(RandFloat() * 2 * PI_F);
}

inline Vec2 RotateBy2(Vec2 a, float rotation)
{
	float sinTheta = sinf(rotation), cosTheta = cosf(rotation);
	return Vec2(cosTheta * a.x - sinTheta * a.y, sinTheta * a.x + cosTheta * a.y);
}

inline Vec2 RotateTowardsNorm2(Vec2 currentDir, Vec2 desiredDir, float moveAmount)
{
	float currentRotation = atan2f(currentDir.y, currentDir.x);

	currentRotation -= (roundf(ModF(atan2f(desiredDir.y, desiredDir.x) - currentRotation, PI_F * 2) / (PI_F * 2)) * 2 - 1) * moveAmount;

	return Vec2(cosf(currentRotation), sinf(currentRotation));
}

inline Vec2 RotateTowards2(Vec2 currentDir, Vec2 desiredDir, float moveAmount)
{
	return RotateTowardsNorm2(currentDir, desiredDir, moveAmount) * glm::length(currentDir);
}

inline Vec2 FromTo2(Vec2 from, Vec2 to, float travelDistance, float desiredDistance = 0) // Doesn't overshoot.
{
	float currentDistance = glm::distance(from, to) + 0.0001f; // Add an epsilon to avoid divide by 0 errors.
	return travelDistance + desiredDistance < currentDistance ? from + (to - from) * (travelDistance / currentDistance) :
		to + (from - to) * (desiredDistance / currentDistance);
}

inline Vec2 TryAddV2(Vec2 original, Vec2 additional, float maxMagnitude)
{
	original += additional;
	float sqrMagnitude = glm::length2(original);
	if (sqrMagnitude > maxMagnitude * maxMagnitude)
		return original / sqrtf(sqrMagnitude) * maxMagnitude;
	return original;
}

inline Vec2 TryAdd2V2(Vec2 original, Vec2 additional, float maxMagnitude)
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

#pragma region Vec3 functions
inline iVec3 ToIV3(Vec3 a)
{
	return iVec3(floorf(a.x), floorf(a.y), floorf(a.z));
}

bool IsNan(Vec3 v)
{
	return glm::isnan(v.x) || glm::isnan(v.y) || glm::isnan(v.z);
}

bool IsInf(Vec3 v)
{
	return glm::isinf(v.x) || glm::isinf(v.y) || glm::isinf(v.z);
}

bool IsUnreal(Vec3 v)
{
	return IsNan(v) || IsInf(v);
}

inline Vec3 RotateLeft(Vec3 a)
{
	return Vec3(-a.y, a.x, a.z);
}

inline Vec3 RotateLeft(Vec3 a, int amount)
{
	int rotation = JMod(amount, 4);
	return Vec3(a.x * int(rotation == 0) - a.y * int(rotation == 1) - a.x * int(rotation = 2) + a.y * int(rotation == 3),
		a.y * int(rotation == 0) + a.x * int(rotation == 1) - a.y * int(rotation = 2) - a.x * int(rotation == 3), a.z);
}

inline Vec3 RotateRight(Vec3 a)
{
	return Vec3(a.y, -a.x, a.z);
}

inline Vec3 RotateRight(Vec3 a, int amount)
{
	return RotateLeft(a, -amount);
}

inline Vec3 RotateRight45(Vec3 a)
{
	return Vec3((a.x + a.y) / 1.41f, (a.y - a.x) / 1.41f, a.z);
}

inline Vec3 Normalized(Vec3 a) // Safe version of glm::normalize
{
	a = glm::normalize(a);
	return IsUnreal(a) ? vZero : a;
}

inline iVec3 Rormalized(Vec3 a)
{
	Vec3 normalized = Normalized(a);
	return iVec3(static_cast<int>(roundf(normalized.x)), static_cast<int>(roundf(normalized.y)), static_cast<int>(roundf(normalized.z)));
}

inline Vec3 V3Min(Vec3 a, Vec3 b)
{
	return glm::length2(a) < glm::length2(b) ? a : b;
}

inline Vec3 V3Max(Vec3 a, Vec3 b)
{
	return glm::length2(a) > glm::length2(b) ? a : b;
}

inline Vec3 Ceil(Vec3 a)
{
	return Vec3(ceilf(a.x), ceilf(a.y), ceilf(a.z));
}

inline Vec3 CircPoint(float yaw, float pitch)
{
	float cosPitch = cos(pitch);
	return Vec3(cos(yaw) * cosPitch, sin(yaw) * cosPitch, sin(pitch));
}

inline Vec3 RandCircPoint()
{
	return CircPoint(RandFloat() * 2 * PI_F, RandFloat() * 2 * PI_F);
}

inline Vec3 RotateTowardsNorm(Vec3 currentDir, Vec3 desiredDir, float moveAmount)
{
	return Vec3(RotateTowardsNorm2(currentDir, desiredDir, moveAmount), 0.f);
	/*
	glm::angleAxis()
	float currentRotation = atan2f(currentDir.y, currentDir.x);

	currentRotation -= (roundf(ModF(atan2f(desiredDir.y, desiredDir.x) - currentRotation, PI_F * 2) / (PI_F * 2)) * 2 - 1) * moveAmount;

	return Vec3(cosf(currentRotation), sinf(currentRotation));*/
}

inline Vec3 RotateTowards(Vec3 currentDir, Vec3 desiredDir, float moveAmount)
{
	return RotateTowardsNorm(currentDir, desiredDir, moveAmount) * glm::length(currentDir);
}

inline Vec3 FromTo(Vec3 from, Vec3 to, float travelDistance, float desiredDistance = 0) // Doesn't overshoot.
{
	float currentDistance = glm::distance(from, to) + 0.0001f; // Add an epsilon to avoid divide by 0 errors.
	return travelDistance + desiredDistance < currentDistance ? from + (to - from) * (travelDistance / currentDistance) :
		to + (from - to) * (desiredDistance / currentDistance);
}

inline Vec3 TryAdd(Vec3 original, Vec3 additional, float maxMagnitude)
{
	original += additional;
	float sqrMagnitude = glm::length2(original);
	if (sqrMagnitude > maxMagnitude * maxMagnitude)
		return original / sqrtf(sqrMagnitude) * maxMagnitude;
	return original;
}

inline Vec3 TryAdd2(Vec3 original, Vec3 additional, float maxMagnitude)
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

glm::mat4 PosScaleMat(Vec3 pos, Vec3 scale = vOne)
{
	glm::mat4 result = glm::mat4(1.0f);
	result = glm::translate(result, pos);
	result = glm::scale(result, scale);
	return result;
}

glm::mat4 PosScaleRotMat(Vec3 pos, Vec3 scale, glm::vec4 rotation)
{
	glm::mat4 result = glm::mat4(1.0f);
	result = glm::translate(result, pos);
	result = glm::rotate(result, rotation.w, (Vec3)rotation);
	result = glm::scale(result, scale);
	return result;
}
#pragma endregion