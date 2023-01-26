#pragma region Basic include stuff
//#define OLC_PGE_APPLICATION
//#include "olcPixelGameEngine.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "FastNoiseLite.h"
#include<string>
#include<set>
#include<map>
#include <chrono>
#include <thread>
#include <iostream>
using namespace std::this_thread;
using namespace std::chrono;
using std::vector;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::max;
using std::min;
using std::remove;
using std::find;
using std::distance;
using std::to_string;
using std::make_unique;
using std::make_shared;

typedef unsigned int uint;
typedef uint8_t byte;
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
int pixelCount = screenWidth * screenHeight;
bool playerAlive = false;
int totalGamePoints;
int psuedoRandomizer = 0;
int frameCount = 0, waveCount = 0;
float tTime = 0.0f;
string deathCauseName = "NULL DEATH CAUSE";

// Very important!
//class Game;
//unique_ptr<Game> game;
#pragma endregion

#pragma region Math
inline float RandFloat()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

inline int JMod(int x, int m)
{
	return ((x % m) + m) % m;
}

inline int Clamp(int value, int minimum, int maximum)
{
	return max(min(value, maximum), minimum);
}

inline int ModClamp(int value, int minimum, int maximum)
{
	return value % (maximum - minimum) + minimum;
}

inline float ClampF(float value, float minimum, float maximum)
{
	return max(min(value, maximum), minimum);
}

inline float ClampF01(float value)
{
	return ClampF(value, 0, 1);
}

inline double ClampD(double value, double minimum, double maximum)
{
	return max(min(value, maximum), minimum);
}

inline double ClampD01(double value)
{
	return ClampD(value, 0, 1);
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

struct Key
{
	bool pressed, held, released;
};

struct Inputs
{
	Key w, a, s, d,
		enter, c, q, e, space,
		up, left, down, right,
		leftMouse, rightMouse, middleMouse;
	//Vec2 mousePosition;
	int mouseScroll = 0;

	Inputs() = default;

	static void ResetKey(Key& key)
	{
		key.pressed = false;
		key.held = false;
		key.released = false;
	}

	static void ReadIntoKey(Key& key, Key newPressings)
	{
		key.held |= newPressings.held;
		key.pressed |= newPressings.pressed;
		key.released |= newPressings.released;
	}

	/*static void UpdateKey(olc::PixelGameEngine* game, Key& key, olc::Key keycode)
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
*/
	void Update2()
	{
		mouseScroll = 0;

		// No resetting of bHeld for wasd or arrow keys as that's covered by the player.
		w.pressed = false;
		w.released = false;

		up.pressed = false;
		up.released = false;
		a.pressed = false;
		a.released = false;

		left.pressed = false;
		left.released = false;

		d.pressed = false;
		d.released = false;

		right.pressed = false;
		right.released = false;

		s.pressed = false;
		s.released = false;

		down.pressed = false;
		down.released = false;

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