#include "Mesh.h"

uint CreateShader(char* vertexShaderSource, char* fragmentShaderSource, string shaderName) // Shader name is entirely in here for debugging.
{
	std::cout << "Generating " << shaderName << '\n';
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

uint defaultShader, framebufferShader, backgroundShader, lineShader, textShader, shadowShader, shadingShader;

vector<std::tuple<std::pair<int, int>, uint*, string>> shaders{ {{DEFAULT_VERT, DEFAULT_FRAG}, &defaultShader, "Default Shader"},
	{{FRAMEBUFFER_VERT, FRAMEBUFFER_FRAG}, &framebufferShader, "Framebuffer Shader"},
	{{BACKGROUND_VERT, BACKGROUND_FRAG}, &backgroundShader, "Background Shader"},
	{{LINE_VERT, LINE_FRAG}, &lineShader, "Line Shader"},
	{{TEXT_VERT, TEXT_FRAG}, &textShader, "Text Shader"},
	{{SHADOW_VERT, SHADOW_FRAG}, &shadowShader, "Shadow Shader"},
	{{SHADING_VERT, SHADING_FRAG}, &shadingShader, "Shading Shader"} };