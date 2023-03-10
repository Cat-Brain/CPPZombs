#pragma region Basic include stuff
#include "resource.h"
#include "FastNoiseLite.h"
#include<string>
#include<set>
#include<map>
#include <chrono>
#include <thread>
#include <iostream>
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
using std::function;

typedef unsigned int uint;
typedef uint8_t byte;
#pragma endregion

#pragma region Global variables
#define PI_F 3.141592f
#define PI_D 3.14159275

#define SQRTTWO_F 1.41421356f
#define SQRTTWO_D 1.41421356237

#define CHUNK_WIDTH 16

uint trueScreenWidth, trueScreenHeight;
float screenRatio;
bool playerAlive = true;
int totalGamePoints;
int psuedoRandomizer = 0;
int frameCount = 0, waveCount = 0;
float tTime = 0.0f;
string deathCauseName = "NULL DEATH CAUSE";
uint totalTexturesCreated = 0;
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

inline float RandFloat()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
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

int PsuedoRandom()
{
	return psuedoRandomizer++;
}

int ScrWidth();
int ScrHeight();