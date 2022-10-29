#include "Includes.h"

const char* defaultVertexShader = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* defaultFragmentShader = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"   FragColor = color;\n"
"}\0";

float vec3Zero[] = { 0.0f, 0.0f, 0.0f };
float vec3One[] = { 1.0f, 1.0f, 1.0f };

byte lowCPUModeBehavior = 1; // 0 = always false, 1 = false when backgrounded else true, 2 = always true.
bool lowCPUMode = false;