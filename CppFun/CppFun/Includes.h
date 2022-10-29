#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <fstream>
#include <cglm/cglm.h>
#include <string>
#include <chrono>
#include <thread>
using namespace std::this_thread;
using namespace std::chrono;

// We also put in a few typedefs here.
typedef uint8_t byte; // A single byte that can be used as an int or a char.
typedef unsigned int uint; // A quickhand for saying unsigned int.