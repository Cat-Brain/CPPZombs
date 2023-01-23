#pragma region Basic include stuff
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "FastNoiseLite.h"
#include<string>
#include<set>
#include<map>
#include <chrono>
#include <thread>
using namespace std::this_thread;
using namespace std::chrono;
using std::max;
using std::min;
using std::vector;
using std::map;
using std::cin;
using std::cout;
using std::remove;
using std::find;
using std::distance;
using std::to_string;
using std::pair;
using std::string;

typedef unsigned int uint;
typedef olc::vi2d Vec2;
typedef olc::vf2d Vec2f;
typedef olc::Pixel Color;
typedef olc::HWButton Key;
#pragma endregion

#pragma region Global variables
#define PI_F 3.141592f
#define PI_D 3.14159275
#define screenWidth 80
#define screenHeight screenWidth
#define screenWidthH (screenWidth >> 1)
#define screenHeightH (screenHeight >> 1)
#define screenWidthHighRes (screenWidth << 2)
#define screenHeightHighRes (screenHeight << 2)
Vec2 screenDim(screenWidth, screenHeight), screenDimH(screenWidthH, screenHeightH);
int pixelCount = screenWidth * screenHeight;

bool playerAlive = false;
Vec2 playerPos(0, 0), lastPlayerPos(0, 0);
Vec2 playerVel(0, 0);
int totalGamePoints;
int psuedoRandomizer = 0;
int frameCount = 0, waveCount = 0;
float tTime = 0.0f;
string deathCauseName = "NULL DEATH CAUSE";

// Very important!
class Game;
Game* game;
#pragma endregion

#pragma region Math
float RandFloat()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

int JMod(int x, int m)
{
	return ((x % m) + m) % m;
}

int Clamp(int value, int minimum, int maximum)
{
	return max(min(value, maximum), minimum);
}

float ClampF(float value, float minimum, float maximum)
{
	return max(min(value, maximum), minimum);
}

float ClampF01(float value)
{
	return ClampF(value, 0, 1);
}

double ClampD(double value, double minimum, double maximum)
{
	return max(min(value, maximum), minimum);
}

double ClampD01(double value)
{
	return ClampD(value, 0, 1);
}

int ModClamp(int value, int minimum, int maximum)
{
	return value % (maximum - minimum) + minimum;
}

Vec2 Vabs(Vec2 a)
{
	return Vec2(abs(a.x), abs(a.y));
}

int Squagnitude(Vec2 a)
{
	return static_cast<int>(max(labs(static_cast<long>(a.x)), labs(static_cast<long>(a.y))));
}

int Diagnitude(Vec2 a)
{
	return abs(a.x) + abs(a.y);
}

float SqrMagnitude(Vec2f a)
{
	return a.x * a.x + a.y * a.y;
}

float Magnitude(Vec2f a)
{
	return std::sqrtf(SqrMagnitude(a));
}

int Squistance(Vec2 a, Vec2 b)
{
	return Squagnitude(a - b);
}

int Diagnistance(Vec2 a, Vec2 b)
{
	return Diagnitude(a - b);
}

Vec2 Squarmalized(Vec2 a)
{
	return a / static_cast<int>(max(1, Squagnitude(a)));
}

Vec2f Normalized(Vec2f a)
{
	return a / max(0.001f, Magnitude(a));
}

Vec2f V2fMin(Vec2f a, Vec2f b)
{
	return { SqrMagnitude(a) < SqrMagnitude(b) ? a : b };
}

float Distance(Vec2f a, Vec2f b)
{
	return Magnitude(a - b);
}

float Dot(Vec2f a, Vec2f b)
{
	return a.x * b.x + a.y * b.y;
}
#pragma endregion

template <typename T>
std::string ToStringWithPrecision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return out.str();
}

int PsuedoRandom()
{
	return psuedoRandomizer++;
}

#pragma region Vec2 functions
Vec2 ToSpace(Vec2 positionInSpace)
{
	return Vec2(positionInSpace.x, screenHeight - positionInSpace.y - 1);
}

Vec2 ToRSpace(Vec2 positionInLocalSpace)
{
	return ToSpace(positionInLocalSpace - playerPos + screenDimH);
}

Vec2 ToSpaceFromR(Vec2 positionInRenderSpace) // May not work.
{
	return ToSpace(positionInRenderSpace) + playerPos - screenDim;
}

// Rotation:
void RotateLeft(Vec2& dir)
{
	dir = Vec2(-dir.y, dir.x);
}

void RotateLeft(Vec2& dir, int amount)
{
	for (int i = 0; i < amount; i++)
		dir = Vec2(-dir.y, dir.x);
}

void RotateRight(Vec2& dir)
{
	dir = Vec2(dir.y, -dir.x);
}

void RotateRight(Vec2& dir, int amount)
{
	for (int i = 0; i < amount; i++)
		dir = Vec2(dir.y, -dir.x);
}

void RotateRight45(Vec2& dir)
{
	dir = Vec2(int((dir.x + dir.y) / 1.41f), int((dir.y - dir.x) / 1.41f));
}
#pragma endregion

class JRGB
{
public:
	byte r, g, b;

	JRGB(byte r = 0, byte g = 0, byte b = 0):
		r(r), g(g), b(b) { }

	JRGB(Color color) :
		r(color.r), g(color.g), b(color.b) { }

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

struct Inputs
{
	Key w, a, s, d,
		enter, c, q, e, space,
		up, left, down, right,
		leftMouse, rightMouse, middleMouse;
	Vec2 mousePosition;
	int mouseScroll = 0;

	Inputs() = default;

	static void ResetKey(Key& key)
	{
		key.bHeld = false;
		key.bPressed = false;
		key.bReleased = false;
	}

	static void ReadIntoKey(Key& key, Key newPressings)
	{
		key.bHeld |= newPressings.bHeld;
		key.bPressed |= newPressings.bPressed;
		key.bReleased |= newPressings.bReleased;
	}

	static void UpdateKey(olc::PixelGameEngine* game, Key& key, olc::Key keycode)
	{
		Key newPressings = game->GetKey(keycode);
		ReadIntoKey(key, newPressings);
	}

	void Update1(olc::PixelGameEngine* game)
	{
		mouseScroll += game->GetMouseWheel() / 120;
		mousePosition = ToSpace(game->GetMousePos() / 4) + playerPos - screenDimH;

		UpdateKey(game, w, olc::W);
		UpdateKey(game, a, olc::A);
		UpdateKey(game, s, olc::S);
		UpdateKey(game, d, olc::D);

		UpdateKey(game, enter, olc::ENTER);
		UpdateKey(game, c, olc::C);
		UpdateKey(game, q, olc::Q);
		UpdateKey(game, e, olc::E);
		UpdateKey(game, space, olc::SPACE);

		UpdateKey(game, up, olc::UP);
		UpdateKey(game, left, olc::LEFT);
		UpdateKey(game, down, olc::DOWN);
		UpdateKey(game, right, olc::RIGHT);

		ReadIntoKey(leftMouse, game->GetMouse(0));
		ReadIntoKey(rightMouse, game->GetMouse(1));
		ReadIntoKey(middleMouse, game->GetMouse(2));
	}

	void Update2()
	{
		mouseScroll = 0;

		// No resetting of bHeld for wasd or arrow keys as that's covered by the player.
		w.bPressed = false;
		w.bReleased = false;

		up.bPressed = false;
		up.bReleased = false;
		a.bPressed = false;
		a.bReleased = false;

		left.bPressed = false;
		left.bReleased = false;

		d.bPressed = false;
		d.bReleased = false;

		right.bPressed = false;
		right.bReleased = false;

		s.bPressed = false;
		s.bReleased = false;

		down.bPressed = false;
		down.bReleased = false;

		Inputs::ResetKey(enter);
		Inputs::ResetKey(c);
		Inputs::ResetKey(q);
		Inputs::ResetKey(e);
		Inputs::ResetKey(space);
		Inputs::ResetKey(leftMouse);
		Inputs::ResetKey(rightMouse);
		Inputs::ResetKey(middleMouse);
	}
};

Vec2 up(0, 1), right(1, 0), down(0, -1), left(-1, 0), vZero(0, 0), vOne(1, 1);