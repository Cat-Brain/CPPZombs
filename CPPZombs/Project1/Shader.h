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

uint defaultShader, framebufferShader, cylinderShader, capsuleShader, textShader, rotatedTextShader, shadowShader, shadingShader,
texturedShader, circleShader, sunShader, coneShader, chunkShader;

vector<std::tuple<std::pair<int, int>, uint*, string>> shaders{
	{{DEFAULT_VERT, DEFAULT_FRAG}, &defaultShader, "Default Shader"},
	{{FRAMEBUFFER_VERT, FRAMEBUFFER_FRAG}, &framebufferShader, "Framebuffer Shader"},
	{{LINE_VERT, CYLINDER_FRAG}, &cylinderShader, "Cylinder Shader"},
	{{LINE_VERT, CAPSULE_FRAG}, &capsuleShader, "Capsule Shader"},
	{{TEXT_VERT, TEXT_FRAG}, &textShader, "Text Shader"},
	{{ROTATED_TEXT_VERT, ROTATED_TEXT_FRAG}, &rotatedTextShader, "Rotated Text Shader"},
	{{SHADOW_VERT, SHADOW_FRAG}, &shadowShader, "Shadow Shader"},
	{{SHADING_VERT, SHADING_FRAG}, &shadingShader, "Shading Shader"},
	{{TEXTURED_VERT, TEXTURED_FRAG}, &texturedShader, "Textured Shader"},
	{{CIRCLE_VERT, CIRCLE_FRAG}, &circleShader, "Circle Shader"},
	{{SUN_VERT, SUN_FRAG}, &sunShader, "Sun Shader"},
	{{LINE_VERT, CONE_FRAG}, &coneShader, "Cone Shader"},
	{{CHUNK_VERT, CHUNK_FRAG}, &chunkShader, "Chunk Shader"},
};