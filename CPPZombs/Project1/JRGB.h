#include "Inputs.h"

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
		return *this * JRGB(scaler, scaler, scaler);
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

	JRGB CLerp(JRGB other, float lerpValue)
	{
		return JRGB(Lerp(int(r), int(other.r), lerpValue), Lerp(int(g), int(other.g), lerpValue), Lerp(int(b), int(other.b), lerpValue));
	}
};

class RGBA
{
public:
	byte r, g, b, a;
	RGBA(byte r = 0, byte g = 0, byte b = 0, byte a = 255) :
		r(r), g(g), b(b), a(a) { }

#pragma region Operators
	RGBA operator +(RGBA other)
	{
		return RGBA(Clamp(r + int(other.r), 0, 255), Clamp(g + int(other.g), 0, 255), Clamp(b + int(other.b), 0, 255));
	}

	RGBA operator +(int scaler)
	{
		return RGBA(Clamp(r + scaler, 0, 255), Clamp(g + scaler, 0, 255), Clamp(b + scaler, 0, 255));
	}

	RGBA operator -(RGBA other)
	{
		return RGBA(Clamp(r - int(other.r), 0, 255), Clamp(g - int(other.g), 0, 255), Clamp(b - int(other.b), 0, 255));
	}

	RGBA operator -(int scaler)
	{
		return *this + -scaler;
	}

	void operator +=(RGBA other)
	{
		*this = *this + other;
	}

	void operator +=(int scaler)
	{
		*this = *this + scaler;
	}

	void operator -=(RGBA other)
	{
		*this = *this - other;
	}

	void operator -=(int scaler)
	{
		*this = *this - scaler;
	}

	RGBA operator *(RGBA other)
	{
		return RGBA(r * other.r, g * other.g, b * other.b);
	}

	RGBA operator *(int scaler)
	{
		return RGBA(r * scaler, g * scaler, b * scaler);
	}

	RGBA operator /(RGBA other)
	{
		return RGBA(r / other.r, g / other.g, b / other.b);
	}

	RGBA operator /(int scaler)
	{
		return RGBA(r / scaler, g / scaler, b / scaler);
	}

	void operator *=(RGBA other)
	{
		*this = *this * other;
	}

	void operator *=(int scaler)
	{
		*this = *this * scaler;
	}

	void operator /=(RGBA other)
	{
		*this = *this / other;
	}

	void operator /=(int scaler)
	{
		*this = *this / scaler;
	}

	bool operator ==(RGBA other)
	{
		return r == other.r && g == other.g && b == other.b;
	}

	bool operator !=(RGBA other)
	{
		return !(*this == other);
	}
#pragma endregion

	static RGBA RandNormalized(byte a = 255)
	{
		Vec3 point = glm::abs(RandCircPoint());
		return RGBA(static_cast<byte>(point.x * 255), static_cast<byte>(point.y * 255), static_cast<byte>(point.z * 255), a);
	}

	bool MaxEq(RGBA newRGB)
	{
		if (r >= newRGB.r && g >= newRGB.g && b >= newRGB.b)
			return false;
		r = max(r, newRGB.r);
		g = max(g, newRGB.g);
		b = max(b, newRGB.b);
		a = max(a, newRGB.a);
		return true;
	}

	bool MinEq(RGBA newRGB)
	{
		if (r <= newRGB.r && g <= newRGB.g && b <= newRGB.b)
			return false;
		r = min(r, newRGB.r);
		g = min(g, newRGB.g);
		b = min(b, newRGB.b);
		a = min(a, newRGB.a);
		return true;
	}

	inline RGBA CLerp(RGBA other, float lerpValue)
	{
		return RGBA(Lerp(int(r), int(other.r), lerpValue), Lerp(int(g), int(other.g), lerpValue), Lerp(int(b), int(other.b), lerpValue), Lerp(int(a), int(other.a), lerpValue));
	}

	inline JRGB ToJRGB()
	{
		return JRGB(r, g, b);
	}
};