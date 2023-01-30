#include "Include.h"

class iVec2
{
public:
	int x, y;

	iVec2(int x = 0, int y = 0) :
		x(x), y(y) { }


	inline iVec2 RotateLeft()
	{
		return iVec2(-y, x);
	}

	iVec2 RotateLeft(int amount)
	{
		int rotation = JMod(amount, 4);
		return iVec2(x * int(rotation == 0) - y * int(rotation == 1) - x * int(rotation = 2) + y * int(rotation == 3),
			y * int(rotation == 0) + x * int(rotation == 1) - y * int(rotation = 2) - x * int(rotation == 3));
	}

	inline iVec2 RotateRight()
	{
		return iVec2(y, -x);
	}

	iVec2 RotateRight(int amount)
	{
		int rotation = JMod(amount, 4);
		return iVec2(x * int(rotation == 0) + y * int(rotation == 1) - x * int(rotation = 2) - y * int(rotation == 3),
			y * int(rotation == 0) - x * int(rotation == 1) - y * int(rotation = 2) + x * int(rotation == 3));
	}

	inline iVec2 RotateRight45()
	{
		return iVec2(int((x + y) / 1.41f), int((y - x) / 1.41f));
	}

	inline iVec2 ClampV(iVec2 minimum, iVec2 maximum)
	{
		return iVec2(Clamp(x, minimum.x, maximum.x), Clamp(y, minimum.y, maximum.y));
	}

	inline iVec2 Abs()
	{
		return iVec2(abs(x), abs(y));
	}

	inline int Squagnitude()
	{
		return static_cast<int>(max(labs(static_cast<long>(x)), labs(static_cast<long>(y))));
	}

	inline int Diagnitude()
	{
		return abs(x) + abs(y);
	}

	inline int Squistance(iVec2 other)
	{
		return (*this - other).Squagnitude();
	}

	inline int Diagnistance(iVec2 other)
	{
		return (*this - other).Diagnitude();
	}

	inline iVec2 Squarmalized()
	{
		return *this / max(1, Squagnitude());
	}


	iVec2 operator+(iVec2 other)
	{
		return iVec2(x + other.x, y + other.y);
	}
	iVec2 operator+(int scaler)
	{
		return iVec2(x + scaler, y + scaler);
	}

	iVec2 operator-(iVec2 other)
	{
		return iVec2(x - other.x, y - other.y);
	}
	iVec2 operator-(int scaler)
	{
		return iVec2(x - scaler, y - scaler);
	}

	iVec2 operator*(iVec2 other)
	{
		return iVec2(x * other.x, y * other.y);
	}
	iVec2 operator*(int scaler)
	{
		return iVec2(x * scaler, y * scaler);
	}

	iVec2 operator/(iVec2 other)
	{
		return iVec2(x / other.x, y / other.y);
	}
	iVec2 operator/(int scaler)
	{
		return iVec2(x / scaler, y / scaler);
	}


	iVec2 operator+=(iVec2 other)
	{
		*this = *this + other;
		return *this;
	}
	iVec2 operator+=(int scaler)
	{
		*this = *this + scaler;
		return *this;
	}

	iVec2 operator-=(iVec2 other)
	{
		*this = *this - other;
		return *this;
	}
	iVec2 operator-=(int scaler)
	{
		*this = *this - scaler;
		return *this;
	}

	iVec2 operator*=(iVec2 other)
	{
		*this = *this * other;
		return *this;
	}
	iVec2 operator*=(int scaler)
	{
		*this = *this * scaler;
		return *this;
	}

	iVec2 operator/=(iVec2 other)
	{
		*this = *this / other;
		return *this;
	}
	iVec2 operator/=(int scaler)
	{
		*this = *this / scaler;
		return *this;
	}

	bool operator==(iVec2 other)
	{
		return x == other.x && y == other.y;
	}
	bool operator!=(iVec2 other)
	{
		return !(*this == other);
	}
};

class Vec2
{
public:
	float x, y;

	Vec2(float x = 0.0f, float y = 0.0f) :
		x(x), y(y) { }

	Vec2(iVec2 pos) :
		x(static_cast<float>(pos.x)), y(static_cast<float>(pos.y)) { }


	Vec2 RotateLeft()
	{
		return Vec2(-y, x);
	}

	Vec2 RotateLeft(int amount)
	{
		int rotation = JMod(amount, 4);
		return Vec2(x * int(rotation == 0) - y * int(rotation == 1) - x * int(rotation = 2) + y * int(rotation == 3),
			y * int(rotation == 0) + x * int(rotation == 1) - y * int(rotation = 2) - x * int(rotation == 3));
	}

	Vec2 RotateRight()
	{
		return Vec2(y, -x);
	}

	Vec2 RotateRight(int amount)
	{
		int rotation = JMod(amount, 4);
		return Vec2(x * int(rotation == 0) + y * int(rotation == 1) - x * int(rotation = 2) - y * int(rotation == 3),
			y * int(rotation == 0) - x * int(rotation == 1) - y * int(rotation = 2) + x * int(rotation == 3));
	}

	Vec2 RotateRight45()
	{
		return Vec2((x + y) / 1.41f, (y - x) / 1.41f);
	}

	inline Vec2 ClampV(Vec2 minimum, Vec2 maximum)
	{
		return Vec2(ClampF(x, minimum.x, maximum.x), ClampF(y, minimum.y, maximum.y));
	}

	inline Vec2 Abs()
	{
		return Vec2(abs(x), abs(y));
	}

	inline float SqrMagnitude()
	{
		return x * x + y * y;
	}

	inline float Magnitude()
	{
		return std::sqrtf(SqrMagnitude());
	}

	inline Vec2 Normalized()
	{
		return *this / max(0.001f, Magnitude());
	}

	inline Vec2 V2fMin(Vec2 other)
	{
		return SqrMagnitude() < other.SqrMagnitude() ? *this : other;
	}

	inline float Distance(Vec2 other)
	{
		return (*this - other).Magnitude();
	}

	inline float Dot(Vec2 other)
	{
		return x * other.x + y * other.y;
	}

	inline Vec2 Ceil()
	{
		return Vec2(ceilf(x), ceilf(y));
	}


	Vec2 operator+(Vec2 other)
	{
		return Vec2(x + other.x, y + other.y);
	}
	Vec2 operator+(float scaler)
	{
		return Vec2(x + scaler, y + scaler);
	}

	Vec2 operator-(Vec2 other)
	{
		return Vec2(x - other.x, y - other.y);
	}
	Vec2 operator-(float scaler)
	{
		return Vec2(x - scaler, y - scaler);
	}

	Vec2 operator*(Vec2 other)
	{
		return Vec2(x * other.x, y * other.y);
	}
	Vec2 operator*(float scaler)
	{
		return Vec2(x * scaler, y * scaler);
	}

	Vec2 operator/(Vec2 other)
	{
		return Vec2(x / other.x, y / other.y);
	}
	Vec2 operator/(float scaler)
	{
		return Vec2(x / scaler, y / scaler);
	}


	Vec2 operator+=(Vec2 other)
	{
		*this = *this + other;
		return *this;
	}
	Vec2 operator+=(float scaler)
	{
		*this = *this + scaler;
		return *this;
	}

	Vec2 operator-=(Vec2 other)
	{
		*this = *this - other;
		return *this;
	}
	Vec2 operator-=(float scaler)
	{
		*this = *this - scaler;
		return *this;
	}

	Vec2 operator*=(Vec2 other)
	{
		*this = *this * other;
		return *this;
	}
	Vec2 operator*=(float scaler)
	{
		*this = *this * scaler;
		return *this;
	}

	Vec2 operator/=(Vec2 other)
	{
		*this = *this / other;
		return *this;
	}
	Vec2 operator/=(float scaler)
	{
		*this = *this / scaler;
		return *this;
	}

	bool operator==(Vec2 other)
	{
		return x == other.x && y == other.y;
	}
	bool operator!=(Vec2 other)
	{
		return !(*this == other);
	}

	operator iVec2()
	{
		return iVec2(static_cast<int>(x), static_cast<int>(y));
	}
};

iVec2 up(0, 1), right(1, 0), down(0, -1), left(-1, 0), vZero(0, 0), vOne(1, 1);