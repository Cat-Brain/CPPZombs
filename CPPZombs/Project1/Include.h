#pragma region Basic include stuff
#include "resource.h"
#include "FastNoiseLite.h"
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <set>
#include <map>
#include <chrono>
#include <thread>
#include <ft2build.h>
#include <Windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include FT_FREETYPE_H

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
using std::function;

typedef unsigned int uint;
typedef uint8_t byte; // Unsigned byte
typedef int8_t sByte; // Signed byte
#pragma endregion

#pragma region Global variables
#define PI_F 3.141592f
#define PI_D 3.14159275

#define SQRTTWO_F 1.41421356f
#define SQRTTWO_D 1.41421356237

#define CHUNK_WIDTH 16

uint trueScreenWidth, trueScreenHeight;
float screenRatio; // Width / height
bool playerAlive = true;
int totalGamePoints;
int psuedoRandomizer = 0;
int frameCount = 0, waveCount = 0;
float tTime = 0, tTime2 = 0;
string deathCauseName = "NULL DEATH CAUSE";
string deathName = "NULL DEATH NAME";

// Very important!
class Game;
unique_ptr<Game> game;
#pragma endregion

#pragma region Math
template <typename T>
inline T Lerp(T a, T b, float l)
{
	return a + T((b - a) * l);
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

inline int CeilToInt(float f)
{
	return static_cast<int>(ceilf(f));
}

inline float RandFloat() // Float from 0-1
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

inline int RandRangeInt(int min, int max) // Inclusive min and inclusive max
{
	return rand() % (max - min + 1) + min;
}

template <typename T>
inline T JMod(T x, T m)
{
	return ((x % m) + m) % m;
}

inline float ModF(float x, float m)
{
	return fmodf((fmodf(x, m) + m), m);
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

string ToStringBool(bool b)
{
	return b ? "true" : "false";
}

std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

template <typename E>
constexpr typename std::underlying_type<E>::type UnEnum(E e) noexcept {
	return static_cast<typename std::underlying_type<E>::type>(e);
}

void ErrorHandle(string message)
{
	printf("Uh oh! There seems to have been a crash!\n\
I know of this crash and am likely trying to track it down, sorry this happened.\n\
The crash type is: %s\n", message.c_str());
	ShowWindow(GetConsoleWindow(), SW_SHOW);
}

int PsuedoRandom()
{
	return psuedoRandomizer++;
}

int ScrWidth();
int ScrHeight();