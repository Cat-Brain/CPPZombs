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

#pragma region Default shader
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
#pragma endregion

#pragma region Framebuffer shader
const char* framebufferVert = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"\n"
"out vec2 uv;\n"
"uniform float currentScrRat, newScrRat;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0, 1.0);\n"
"	uv = vec2((aPos.x + 1) * newScrRat / currentScrRat - 1, aPos.y) * 0.5 + 0.5;\n"
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
#pragma endregion

#pragma region Background shader
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
"uniform vec3 col1, col2;\n"
""
"vec2 hash(vec2 p) // replace this by something better\n"
"{\n"
"	p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));\n"
"	return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);\n"
"}\n"
"\n"
"float noise(in vec2 p)\n"
"{\n"
"	const float K1 = 0.366025404; // (sqrt(3)-1)/2;\n"
"	const float K2 = 0.211324865; // (3-sqrt(3))/6;\n"
"\n"
"	vec2  i = floor(p + (p.x + p.y) * K1);\n"
"	vec2  a = p - i + (i.x + i.y) * K2;\n"
"	float m = step(a.y, a.x);\n"
"	vec2  o = vec2(m, 1.0 - m);\n"
"	vec2  b = a - o + K2;\n"
"	vec2  c = a - 1.0 + 2.0 * K2;\n"
"	vec3  h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);\n"
"	vec3  n = h * h * h * h * vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));\n"
"	return dot(n, vec3(70.0));\n"
"}\n"
"\n"
"void main()"
"{\n"
"	\n"
"	vec2 uv = offset + fragPos * screenDim;\n"
"	uv *= 0.025;\n"
"	mat2 m = mat2(1.6, 1.2, -1.2, 1.6);\n"
"	float f = 0.5000 * noise(uv); uv = m * uv;\n"
"	f += 0.2500 * noise(uv); uv = m * uv;\n"
"	f += 0.1250 * noise(uv); uv = m * uv;\n"
"	f += 0.0625 * noise(uv); uv = m * uv;\n"
"	FragColor = vec4(mix(col1, col2, f), 1.0);\n"
"}\0";

uint backgroundShader;
#pragma endregion

#pragma region Default shader
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
#pragma endregion

#pragma region Text shader
const char* textVert = "#version 330 core\n"
"layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
"out vec2 TexCoords;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(vertex.xy, 0.0, 1.0); \n"
"	TexCoords = vertex.zw; \n"
"}\0";

const char* textFrag = "#version 330 core\n"
"in vec2 TexCoords; \n"
"out vec4 color;\n"
"\n"
"uniform sampler2D text;\n"
"uniform vec4 textColor;\n"
"\n"
"void main()\n"
"{\n"
"	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
"	color = textColor * sampled;\n"
"};\0";

uint textShader;
#pragma endregion

#pragma region Shadow shader
const char* shadowVert =
"layout (location = 0) in vec2 aPos;\n"
"\n"
"uniform vec2 center, scrDim;\n"
"uniform int range;\n"
"out vec2 pos;\n"
"\n"
"void main()\n"
"{\n"
"	pos = aPos * 2 * range + center - range;\n"
"   gl_Position = vec4(pos / scrDim, 0.0, 1.0);\n"
"}\0";

const char* shadowFrag = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 pos;\n"
"uniform vec2 center, scrDim;\n"
"\n"
"uniform sampler2D subScat;\n"
"\n"
"void main()"
"{\n"
"	\n"
"	FragColor = texture(subScat, pos / scrDim);\n"
"}\0";

uint shadowShader;
#pragma endregion

#pragma region Shading shader
const char* shadingVert = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"\n"
"out vec2 shadUV, uv;\n"
"uniform vec2 screenDim, pos;\n"
"uniform float currentScrRat, newScrRat;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0, 1.0);\n"
"	shadUV = (aPos * screenDim + pos) / screenDim;\n"
"	uv = aPos;\n"
"}\0";

const char* shadingFrag = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 shadUV, uv;\n"
"\n"
"uniform sampler2D shadowTexture, screenTexture;"
"\n"
"void main()"
"{\n"
"	FragColor = texture(shadowTexture, shadUV) * texture(screenTexture, uv);\n"
"}\0";

uint shadingShader;
#pragma endregion

vector<std::pair<std::pair<const char*, const char*>, uint*>> shaders{ {{defaultVert, defaultFrag}, &defaultShader},
	{{framebufferVert, framebufferFrag}, &framebufferShader},
	{{backgroundVert, backgroundFrag}, &backgroundShader},
	{{lineVert, lineFrag}, &lineShader},
	{{textVert, textFrag}, &textShader},
	{{shadowVert, shadowFrag}, &shadowShader},
	{{shadingVert, shadingFrag}, &shadingShader} };