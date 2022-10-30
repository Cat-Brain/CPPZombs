#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using std::vector;
using std::map;
using std::cin;
using std::cout;

typedef unsigned int uint;
typedef olc::vi2d Vec2;
typedef olc::Pixel Color;
typedef olc::HWButton button;

int screenWidth = 120, screenHeight = 75;

int JMod(int x, int m)
{
	return ((x % m) + m) % m;
}

struct Inputs
{
	button w, a, s, d,
		up, left, down, right,
		leftMouse, rightMouse, middleMouse;

	Inputs(button w, button a, button s, button d,
		button up, button left, button down, button right,
		button leftMouse, button rightMouse, button middleMouse):
		w(w), a(a), s(s), d(d), up(up), left(left), down(down), right(right), leftMouse(leftMouse), rightMouse(rightMouse), middleMouse(middleMouse)
	{ }
};