#include "Vec2.h"

class JRGB
{
public:
	byte r, g, b;

	JRGB(byte r = 0, byte g = 0, byte b = 0) :
		r(r), g(g), b(b) { }

#pragma region Operators
	JRGB operator +(JRGB other)
	{
		return JRGB(Clamp(r + int(other.r), 0, 255), Clamp(g + int(other.g), 0, 255), Clamp(b + int(other.b), 0, 255));
	}

	JRGB operator +(int scaler)
	{
		return JRGB(Clamp(r + scaler, 0, 255), Clamp(g + scaler, 0, 255), Clamp(b + scaler, 0, 255));
	}

	JRGB operator -(JRGB other)
	{
		return JRGB(Clamp(r - int(other.r), 0, 255), Clamp(g - int(other.g), 0, 255), Clamp(b - int(other.b), 0, 255));
	}

	JRGB operator -(int scaler)
	{
		return *this + -scaler;
	}

	void operator +=(JRGB other)
	{
		*this = *this + other;
	}

	void operator +=(int scaler)
	{
		*this = *this + scaler;
	}

	void operator -=(JRGB other)
	{
		*this = *this - other;
	}

	void operator -=(int scaler)
	{
		*this = *this - scaler;
	}

	JRGB operator *(JRGB other)
	{
		return JRGB(min(r * other.r, 255), min(g * other.g, 255), min(b * other.b, 255));
	}

	JRGB operator *(int scaler)
	{
		return JRGB(min(r * scaler, 255), min(g * scaler, 255), min(b * scaler, 255));
	}

	JRGB operator /(JRGB other)
	{
		return JRGB(r / other.r, g / other.g, b / other.b);
	}

	JRGB operator /(int scaler)
	{
		return JRGB(r / scaler, g / scaler, b / scaler);
	}

	void operator *=(JRGB other)
	{
		*this = *this * other;
	}

	void operator *=(int scaler)
	{
		*this = *this * scaler;
	}

	void operator /=(JRGB other)
	{
		*this = *this / other;
	}

	void operator /=(int scaler)
	{
		*this = *this / scaler;
	}

	bool operator ==(JRGB other)
	{
		return r == other.r && g == other.g && b == other.b;
	}

	bool operator !=(JRGB other)
	{
		return !(*this == other);
	}
#pragma endregion

	bool MaxEq(JRGB newRGB)
	{
		if (r >= newRGB.r && g >= newRGB.g && b >= newRGB.b)
			return false;
		r = max(r, newRGB.r);
		g = max(g, newRGB.g);
		b = max(b, newRGB.b);
		return true;
	}

	bool MinEq(JRGB newRGB)
	{
		if (r <= newRGB.r && g <= newRGB.g && b <= newRGB.b)
			return false;
		r = min(r, newRGB.r);
		g = min(g, newRGB.g);
		b = min(b, newRGB.b);
		return true;
	}
};