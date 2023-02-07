#include "Resources.h"

class Vec2
{
public:
	int x, y;

	Vec2(int x = 0, int y = 0) :
		x(x), y(y) { }


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
	Vec2 operator+(int scaler)
	{
		return Vec2(x + scaler, y + scaler);
	}

	Vec2 operator-(Vec2 other)
	{
		return Vec2(x - other.x, y - other.y);
	}
	Vec2 operator-(int scaler)
	{
		return Vec2(x - scaler, y - scaler);
	}

	Vec2 operator*(Vec2 other)
	{
		return Vec2(x * other.x, y * other.y);
	}
	Vec2 operator*(int scaler)
	{
		return Vec2(x * scaler, y * scaler);
	}

	Vec2 operator/(Vec2 other)
	{
		return Vec2(x / other.x, y / other.y);
	}
	Vec2 operator/(int scaler)
	{
		return Vec2(x / scaler, y / scaler);
	}

	Vec2 operator%(Vec2 other)
	{
		return Vec2(x % other.x, y % other.y);
	}
	Vec2 operator%(int scaler)
	{
		return Vec2(x % scaler, y % scaler);
	}


	Vec2 operator+=(Vec2 other)
	{
		*this = *this + other;
		return *this;
	}
	Vec2 operator+=(int scaler)
	{
		*this = *this + scaler;
		return *this;
	}

	Vec2 operator-=(Vec2 other)
	{
		*this = *this - other;
		return *this;
	}
	Vec2 operator-=(int scaler)
	{
		*this = *this - scaler;
		return *this;
	}

	Vec2 operator*=(Vec2 other)
	{
		*this = *this * other;
		return *this;
	}
	Vec2 operator*=(int scaler)
	{
		*this = *this * scaler;
		return *this;
	}

	Vec2 operator/=(Vec2 other)
	{
		*this = *this / other;
		return *this;
	}
	Vec2 operator/=(int scaler)
	{
		*this = *this / scaler;
		return *this;
	}

	Vec2 operator%=(Vec2 other)
	{
		*this = *this % other;
		return *this;
	}
	Vec2 operator%=(int scaler)
	{
		*this = *this % scaler;
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
};

class Vec2f
{
public:
	float x, y;

	Vec2f(float x = 0.0f, float y = 0.0f) :
		x(x), y(y) { }

	Vec2f(Vec2 pos) :
		x(static_cast<float>(pos.x)), y(static_cast<float>(pos.y)) { }


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
		return Vec2f((x + y) / 1.41f, (y - x) / 1.41f);
	}

	Vec2f Rotate(float theta)
	{
		float cs = cos(theta);
		float sn = sin(theta);

		return { x * cs - y * sn, x * sn + y * cs };
	}

	inline Vec2f ClampV(Vec2f minimum, Vec2f maximum)
	{
		return Vec2f(ClampF(x, minimum.x, maximum.x), ClampF(y, minimum.y, maximum.y));
	}

	inline Vec2f Abs()
	{
		return Vec2f(abs(x), abs(y));
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

	inline Vec2 Rormalized()
	{
		Vec2f normalized = Normalized();
		return Vec2(static_cast<int>(roundf(normalized.x)), static_cast<int>(roundf(normalized.y)));
	}

	inline Vec2f V2fMin(Vec2f other)
	{
		return SqrMagnitude() < other.SqrMagnitude() ? *this : other;
	}

	inline float Distance(Vec2f other)
	{
		return (*this - other).Magnitude();
	}

	inline float Dot(Vec2f other)
	{
		return x * other.x + y * other.y;
	}

	inline Vec2f Ceil()
	{
		return Vec2f(ceilf(x), ceilf(y));
	}


	Vec2f operator+(Vec2f other)
	{
		return Vec2f(x + other.x, y + other.y);
	}
	Vec2f operator+(float scaler)
	{
		return Vec2f(x + scaler, y + scaler);
	}

	Vec2f operator-(Vec2f other)
	{
		return Vec2f(x - other.x, y - other.y);
	}
	Vec2f operator-(float scaler)
	{
		return Vec2f(x - scaler, y - scaler);
	}

	Vec2f operator*(Vec2f other)
	{
		return Vec2f(x * other.x, y * other.y);
	}
	Vec2f operator*(float scaler)
	{
		return Vec2f(x * scaler, y * scaler);
	}

	Vec2f operator/(Vec2f other)
	{
		return Vec2f(x / other.x, y / other.y);
	}
	Vec2f operator/(float scaler)
	{
		return Vec2f(x / scaler, y / scaler);
	}


	Vec2f operator+=(Vec2f other)
	{
		*this = *this + other;
		return *this;
	}
	Vec2f operator+=(float scaler)
	{
		*this = *this + scaler;
		return *this;
	}

	Vec2f operator-=(Vec2f other)
	{
		*this = *this - other;
		return *this;
	}
	Vec2f operator-=(float scaler)
	{
		*this = *this - scaler;
		return *this;
	}

	Vec2f operator*=(Vec2f other)
	{
		*this = *this * other;
		return *this;
	}
	Vec2f operator*=(float scaler)
	{
		*this = *this * scaler;
		return *this;
	}

	Vec2f operator/=(Vec2f other)
	{
		*this = *this / other;
		return *this;
	}
	Vec2f operator/=(float scaler)
	{
		*this = *this / scaler;
		return *this;
	}

	bool operator==(Vec2f other)
	{
		return x == other.x && y == other.y;
	}
	bool operator!=(Vec2f other)
	{
		return !(*this == other);
	}

	operator Vec2()
	{
		return Vec2(static_cast<int>(x), static_cast<int>(y));
	}
};

Vec2 up(0, 1), right(1, 0), down(0, -1), left(-1, 0), vZero(0, 0), vOne(1, 1);