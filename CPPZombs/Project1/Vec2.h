#include "Include.h"

class Vec2
{
public:
	int x, y;

	Vec2(int x, int y) :
		x(x), y(y) { }

	Vec2(int both):
		x(both), y(both) { }

	inline Vec2 ToSpace()
	{
		return Vec2(x, screenHeight - y - 1);
	}

	inline Vec2 ToRSpace();

	inline Vec2 ToSpaceFromR();


	inline Vec2 RotateLeft()
	{
		return Vec2(-y, x);
	}

	Vec2 RotateLeft(int amount)
	{
		int rotation = JMod(amount, 4);
		return Vec2(x * int(rotation == 0) - y * int(rotation == 1) - x * int(rotation = 2) + y * int(rotation == 3),
			y * int(rotation == 0) + x * int(rotation == 1) - y * int(rotation = 2) - x * int(rotation == 3));
	}

	inline Vec2 RotateRight()
	{
		return Vec2(y, -x);
	}

	Vec2 RotateRight(int amount)
	{
		int rotation = JMod(amount, 4);
		return Vec2(x * int(rotation == 0) + y * int(rotation == 1) - x * int(rotation = 2) - y * int(rotation == 3),
			y * int(rotation == 0) - x * int(rotation == 1) - y * int(rotation = 2) + x * int(rotation == 3));
	}

	inline Vec2 RotateRight45()
	{
		return Vec2(int((x + y) / 1.41f), int((y - x) / 1.41f));
	}

	inline Vec2 ClampV(Vec2 minimum, Vec2 maximum)
	{
		return Vec2(Clamp(x, minimum.x, maximum.x), Clamp(y, minimum.y, maximum.y));
	}

	inline Vec2 Abs()
	{
		return Vec2(abs(x), abs(y));
	}

	inline int Squagnitude()
	{
		return static_cast<int>(max(labs(static_cast<long>(x)), labs(static_cast<long>(y))));
	}

	inline int Diagnitude()
	{
		return abs(x) + abs(y);
	}

	inline int Squistance(Vec2 other)
	{
		return (*this - other).Squagnitude();
	}

	inline int Diagnistance(Vec2 other)
	{
		return (*this - other).Diagnitude();
	}

	inline Vec2 Squarmalized()
	{
		return *this / max(1, Squagnitude());
	}


	Vec2 operator+(Vec2 other)
	{
		return Vec2(x + other.x, y + other.y);
	}

	Vec2 operator-(Vec2 other)
	{
		return Vec2(x - other.x, y - other.y);
	}

	Vec2 operator*(Vec2 other)
	{
		return Vec2(x * other.x, y * other.y);
	}

	Vec2 operator/(Vec2 other)
	{
		return Vec2(x / other.x, y / other.y);
	}
};

class Vec2f
{
public:
	float x, y;

	Vec2f(float x, float y) :
		x(x), y(y) { }

	inline Vec2f ToSpace()
	{
		return Vec2f(x, screenHeight - y - 1);
	}

	inline Vec2f ToRSpace();

	inline Vec2f ToSpaceFromR();


	Vec2f RotateLeft()
	{
		return Vec2f(-y, x);
	}

	Vec2f RotateLeft(int amount)
	{
		int rotation = JMod(amount, 4);
		return Vec2f(x * int(rotation == 0) - y * int(rotation == 1) - x * int(rotation = 2) + y * int(rotation == 3),
			y * int(rotation == 0) + x * int(rotation == 1) - y * int(rotation = 2) - x * int(rotation == 3));
	}

	Vec2f RotateRight()
	{
		return Vec2f(y, -x);
	}

	Vec2f RotateRight(int amount)
	{
		int rotation = JMod(amount, 4);
		return Vec2f(x * int(rotation == 0) + y * int(rotation == 1) - x * int(rotation = 2) - y * int(rotation == 3),
			y * int(rotation == 0) - x * int(rotation == 1) - y * int(rotation = 2) + x * int(rotation == 3));
	}

	Vec2f RotateRight45()
	{
		return Vec2f(int((x + y) / 1.41f), int((y - x) / 1.41f));
	}

	inline float SqrMagnitude()
	{
		return x * x + y * y;
	}

	inline float Magnitude()
	{
		return std::sqrtf(SqrMagnitude());
	}

	inline Vec2f Normalized()
	{
		return *this / max(0.001f, Magnitude());
	}

	inline Vec2f V2fMin(Vec2f other)
	{
		return { SqrMagnitude() < other.SqrMagnitude() ? *this : other };
	}

	inline float Distance(Vec2f other)
	{
		return (*this - other).Magnitude();
	}

	inline float Dot(Vec2f other)
	{
		return x * other.x + y * other.y;
	}


	Vec2f operator+(Vec2 other)
	{
		return Vec2f(x + other.x, y + other.y);
	}
	Vec2f operator+(Vec2f other)
	{
		return Vec2f(x + other.x, y + other.y);
	}

	Vec2f operator-(Vec2 other)
	{
		return Vec2f(x - other.x, y - other.y);
	}
	Vec2f operator-(Vec2f other)
	{
		return Vec2f(x - other.x, y - other.y);
	}

	Vec2f operator*(Vec2 other)
	{
		return Vec2f(x * other.x, y * other.y);
	}
	Vec2f operator*(Vec2f other)
	{
		return Vec2f(x * other.x, y * other.y);
	}

	Vec2f operator/(Vec2 other)
	{
		return Vec2f(x / other.x, y / other.y);
	}
	Vec2f operator/(Vec2f other)
	{
		return Vec2f(x / other.x, y / other.y);
	}
};

Vec2 up(0, 1), right(1, 0), down(0, -1), left(-1, 0), vZero(0, 0), vOne(1, 1);
Vec2 screenDim(screenWidth, screenHeight), screenDimH(screenWidthH, screenHeightH);

Vec2 playerPos(0, 0), lastPlayerPos(0, 0);
Vec2 playerVel(0, 0);

inline Vec2 Vec2::ToRSpace()
{
	return (*this - playerPos + screenDimH).ToSpace();
}

inline Vec2 Vec2::ToSpaceFromR() // May not work.
{
	return ToSpace() + playerPos - screenDim;
}

inline Vec2f Vec2f::ToRSpace()
{
	return (*this - playerPos + screenDimH).ToSpace();
}

inline Vec2f Vec2f::ToSpaceFromR() // May not work.
{
	return ToSpace() + playerPos - screenDim;
}