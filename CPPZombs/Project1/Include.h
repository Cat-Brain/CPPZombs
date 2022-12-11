#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "FastNoiseLite.h"

#define NDEBUG false
#ifndef DEBUG
#define DEBUG !NDEBUG
#endif

using std::vector;
using std::map;
using std::cin;
using std::cout;
using std::remove;
using std::find;
using std::distance;
using std::to_string;
#include<string>
using std::string;
#include<set>
using std::set;
#include<map>
using std::map;
using std::pair;
#include <chrono>
#include <thread>
using namespace std::this_thread;
using namespace std::chrono;

typedef unsigned int uint;
typedef olc::vi2d Vec2;
typedef olc::vf2d Vec2f;
typedef olc::Pixel Color;
typedef olc::HWButton button;

#define GRID_SIZE 3
#define GRID_AREA GRID_SIZE * GRID_SIZE
int screenWidth = 50, screenHeight = 50,
	screenWidthH = screenWidth >> 1, screenHeightH = screenHeight >> 1,
	screenWidthT = screenWidth * GRID_SIZE, screenHeightT = screenHeight * GRID_SIZE,
	screenWidthTH = screenWidthT >> 1, screenHeightTH = screenHeightT >> 1;
Vec2 screenDim(screenWidth, screenHeight), screenDimH(screenWidthH, screenHeightH),
	screenDimT(screenWidthT, screenHeightT), screenDimTH(screenWidthTH, screenHeightTH);
int pixelCount = screenWidth * screenHeight * GRID_AREA;

#pragma region Math
int JMod(int x, int m)
{
	return ((x % m) + m) % m;
}

int Squagnitude(Vec2 a)
{
	return (int)fmaxf(fabsf(a.x), fabsf(a.y));
}

int Diagnitude(Vec2 a)
{
	return abs(a.x) + abs(a.y);
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
	return a / (int)fmaxf(1, Squagnitude(a));
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
	int mouseScroll;

	Inputs() = default;
};

bool playerAlive = false;
Vec2 playerPos(0, 0), lastPlayerPos(0, 0);
Vec2 playerVel(0, 0);
int totalGamePoints;
int psuedoRandomizer = 0;
int frameCount = 0, waveCount = 0;
float tTime = 0.0f;
string deathCauseName = "NULL DEATH CAUSE";

int PsuedoRandom()
{
	return psuedoRandomizer++;
}

#pragma region Vec2 functions
Vec2 ToSpace(Vec2 positionInWorldSpace)
{
	return Vec2(positionInWorldSpace.x, screenHeight - positionInWorldSpace.y - 1);
}

Vec2 ToSpace2(Vec2 positionInCSpace)
{
	return Vec2(positionInCSpace.x, screenHeight * GRID_SIZE - positionInCSpace.y - 1);
}

Vec2 ToRSpace(Vec2 positionInLocalSpace)
{
	return ToSpace(positionInLocalSpace - playerPos + screenDimH) * GRID_SIZE;
}

Vec2 ToRSpace2(Vec2 positionInLocalSpace)
{
	return ToSpace2(positionInLocalSpace - playerPos * GRID_SIZE + screenDimH * GRID_SIZE);
}

Vec2 ToCSpace(Vec2 positionInEntitySpace)
{
	return positionInEntitySpace * GRID_SIZE;
}

Vec2 ToRandomCSpace(Vec2 positionInEntitySpace) // Randomized within a 3x3.
{
	return positionInEntitySpace * GRID_SIZE + Vec2(rand() % GRID_SIZE, rand() % GRID_SIZE);
}

Vec2 ToESpace(Vec2 positionInCollectibleSpace)
{
	return positionInCollectibleSpace / GRID_SIZE;
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

class Screen : public olc::PixelGameEngine
{
public:
	olc::Sprite screen;
	olc::Sprite bigScreen;

	Inputs inputs;

	Screen() { }

	void DrawScreen()
	{
		DrawSprite(0, 0, &screen, GRID_SIZE);
	}
};

Vec2 up(0, 1), right(1, 0), down(0, -1), left(-1, 0), vZero(0, 0);