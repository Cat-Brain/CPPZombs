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
"\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x * scale.x + position.x, aPos.y * scale.y + position.y, 0.0, 1.0);\n"
"}\0";

const char* defaultFrag = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"	FragColor = color;\n"
"}\0";

uint defaultShader;

const char* framebufferVert = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"\n"
"out vec2 uv;\n"
"uniform float currentScrRat, newScrRat;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0, 1.0);\n"
"	uv = vec2((aPos.x + 1) * currentScrRat / newScrRat - 1, aPos.y) * 0.5 + 0.5;\n"
"}\0";

const char* framebufferFrag = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 uv;\n"
"\n"
"uniform sampler2D screenTexture;"
"\n"
"void main()"
"{\n"
"	FragColor = texture(screenTexture, uv);\n"
"}\0";

uint framebufferShader;

const char* backgroundVert = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"out vec2 fragPos;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);\n"
"	fragPos = aPos;\n"
"}\0";

const char* backgroundFrag = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 fragPos;\n"
"uniform vec2 offset, screenDim;\n"
"\n"
"void main()"
"{\n"
"	float colorValue = (sin((offset.x + fragPos.x * screenDim.x) * 0.1) + sin((offset.y + fragPos.y * screenDim.y) * 0.1)) * 0.25 + 0.5;\n"
"	FragColor = vec4(colorValue, colorValue, colorValue, 1.0);\n"
"}\0";

uint backgroundShader;

const char* lineVert = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"\n"
"uniform vec2 a, b;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(a * aPos.x + b * aPos.y, 0.0, 1.0);\n"
"}\0";

const char* lineFrag = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"	FragColor = color;\n"
"}\0";

uint lineShader;

vector<std::pair<std::pair<const char*, const char*>, uint*>> shaders{ {{defaultVert, defaultFrag}, &defaultShader},
	{{framebufferVert, framebufferFrag}, &framebufferShader},
	{{backgroundVert, backgroundFrag}, &backgroundShader},
	{{lineVert, lineFrag}, &lineShader } };