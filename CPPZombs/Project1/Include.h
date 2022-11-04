#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using std::vector;
using std::map;
using std::cin;
using std::cout;
using std::remove;

typedef unsigned int uint;
typedef olc::vi2d Vec2;
typedef olc::vf2d Vec2f;
typedef olc::Pixel Color;
typedef olc::HWButton button;

int screenWidth = 50, screenHeight = 50,
	screenWidthH = screenWidth >> 1, screenHeightH = screenHeight >> 1;
Vec2 screenDim(screenWidth, screenHeight), screenDimH(screenWidthH, screenHeightH);

int JMod(int x, int m)
{
	return ((x % m) + m) % m;
}

int Squagnitude(Vec2 a)
{
	return (int)fmaxf(fabsf(a.x), fabsf(a.y));
}

int Squistance(Vec2 a, Vec2 b)
{
	return Squagnitude(a - b);
}



struct Inputs
{
	button w, a, s, d,
		up, left, down, right,
		leftMouse, rightMouse, middleMouse;
	Vec2 mousePosition;

	Inputs(button w, button a, button s, button d,
		button up, button left, button down, button right,
		button leftMouse, button rightMouse, button middleMouse, Vec2 mousePosition):
		w(w), a(a), s(s), d(d), up(up), left(left), down(down), right(right), leftMouse(leftMouse), rightMouse(rightMouse), middleMouse(middleMouse), mousePosition(mousePosition)
	{ }
};

Vec2 camPos(0, 0);