#include "Mesh.h"

uint CreateShader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
	uint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	uint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	uint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	glUseProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return shaderProgram;

}

const char* defaultVert = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"\n"
"uniform vec2 scale, position;\n"
"out vec2 uv;"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x * scale.x + position.x, aPos.y * scale.y + position.y, 0.0, 1.0);\n"
"	uv = aPos;\n"
"}\0";

const char* defaultFrag = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 uv;\n"
"\n"
"void main()\n"
"{\n"
"	FragColor = vec4(uv.xy, 1.0f, 1.0f);\n"
"}\0";

uint defaultShader;

const char* framebufferVert = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"\n"
"out vec2 uv;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0, 1.0);\n"
"	uv = aPos * 0.5 + 0.5;\n"
"}\0";

const char* framebufferFrag = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 uv;\n"
"\n"
"uniform sampler2D screenTexture;"
"\n"
"void main()"
"{\n"
"	FragColor = vec4(texture(screenTexture, uv).rgb, 1.0);\n"
"}\0";

uint framebufferShader;

vector<std::pair<std::pair<const char*, const char*>, uint*>> shaders{ {{defaultVert, defaultFrag}, &defaultShader}, {{framebufferVert, framebufferFrag}, &framebufferShader} };