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
typedef olc::HWButton button;
#pragma endregion

// Global variables:
int screenWidth = 80, screenHeight = screenWidth,
	screenWidthH = screenWidth >> 1, screenHeightH = screenHeight >> 1;
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

#pragma region Math
int JMod(int x, int m)
{
	return ((x % m) + m) % m;
}

int Squagnitude(Vec2 a)
{
	return static_cast<int>(max(labs(static_cast<long>(a.x)), labs(static_cast<long>(a.y))));
}

int Diagnitude(Vec2 a)
{
	return abs(a.x) + abs(a.y);
}

float Magnitude(Vec2f a)
{
	return std::sqrtf(a.x * a.x + a.y * a.y);
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
	return a / Magnitude(a);
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

struct Inputs
{
	button w, a, s, d,
		enter, c, q, e, space,
		up, left, down, right,
		leftMouse, rightMouse, middleMouse;
	Vec2 mousePosition;
	int mouseScroll = 0;

	Inputs() = default;
};

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

Vec2 up(0, 1), right(1, 0), down(0, -1), left(-1, 0), vZero(0, 0), vOne(1, 1);