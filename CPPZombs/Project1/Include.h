#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "FastNoiseLite.h"

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

typedef unsigned int uint;
typedef olc::vi2d Vec2;
typedef olc::vf2d Vec2f;
typedef olc::Pixel Color;
typedef olc::HWButton button;

int screenWidth = 50, screenHeight = 50,
	screenWidthH = screenWidth >> 1, screenHeightH = screenHeight >> 1;
Vec2 screenDim(screenWidth, screenHeight), screenDimH(screenWidthH, screenHeightH);
#define GRID_SIZE 3
int pixelCount = screenWidth * screenHeight * GRID_SIZE * GRID_SIZE;

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
	return a.x + a.y;
}

int Squistance(Vec2 a, Vec2 b)
{
	return Squagnitude(a - b);
}

Vec2 Squarmalized(Vec2 a)
{
	return a / (int)fmaxf(1, Squagnitude(a));
}



struct Inputs
{
	button w, a, s, d,
		enter, c, q, e,
		up, left, down, right,
		leftMouse, rightMouse, middleMouse;
	Vec2 mousePosition;

	Inputs() = default;
};

bool playerAlive = false;
Vec2 playerPos(0, 0);
Vec2 playerVel(0, 0);
int totalGamePoints;

Vec2 ToSpace(Vec2 positionInWorldSpace)
{
	return Vec2(positionInWorldSpace.x, screenHeight - positionInWorldSpace.y - 1);
}

Vec2 ToRSpace(Vec2 positionInLocalSpace)
{
	return ToSpace(positionInLocalSpace - playerPos + screenDimH) * 3;
}

class Screen : public olc::PixelGameEngine
{
public:
	olc::Sprite screen;
	olc::Sprite bigScreen;

	Inputs inputs;// = Inputs(button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), Vec2(0, 0));

	Screen() { }

	void DrawScreen()
	{
		DrawSprite(0, 0, &screen, GRID_SIZE);
	}
};