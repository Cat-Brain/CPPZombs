#include "Global variables.h"

template <typename T>
class List
{
public:
	T* ptr;
	uint count;

	void Add(T value)
	{
		T* newPtr = (T*)realloc(ptr, sizeof(T) * ((unsigned long long)count + (unsigned long long)1));
		if (newPtr != NULL)
		{
			ptr = newPtr;
			ptr[count] = value;
			count++;
		}
		else
		{
			printf("COULDN'T ALLOCATE ENOUGH MEMORY.");
		}
	}

	void Extend(uint length)
	{
		T* newPtr = (T*)realloc(ptr, sizeof(T) * (size_t)(count + length));
		if (newPtr != NULL)
		{
			ptr = newPtr;
			count += length;
		}
		else
		{
			printf("COULDN'T ALLOCATE ENOUGH MEMORY.");
		}
	}

	void ExtendByList(List<T> extendedList)
	{
		T* newPtr = (T*)realloc(ptr, sizeof(T) * ((unsigned long long)count + (unsigned long long)extendedList.count));
		if (newPtr != NULL)
		{
			ptr = newPtr;
			for (uint i = 0; i < extendedList.count; i++)
				ptr[i + count] = extendedList.ptr[i];
			count += extendedList.count;
		}
		else
		{
			printf("COULDN'T ALLOCATE ENOUGH MEMORY.");
		}
	}

	void RemoveAt(uint position)
	{
		if (position >= 0 && position < count)
		{
			count--;

			T tempValue = ptr[count - 1];
			ptr[count - 1] = ptr[count];

			T* tempPtr = (T*)realloc(ptr, sizeof(T) * count);

			if (tempPtr != NULL)
			{
				for (uint i = position; i < count - 1; i++)
					ptr[i] = ptr[i + 1];

				ptr = tempPtr;
			}
			else
			{
				printf("COULDN'T ALLOCATE ENOUGH MEMORY.");
				ptr[count - 1] = tempValue;
			}
		}
		else
		{
			printf("THERE IS NO VALUE AT POSITION %u", position);
		}
	}

	void RemoveEnd()
	{
		T* tempPtr = (T*)realloc(ptr, sizeof(T) * (unsigned long long)(count - 1));
		if (tempPtr != NULL)
		{
			count--;
			ptr = realloc(sizeof(T) * count);
		}
		else
		{
			printf("MEMORY ALLOCATION HAS FAILED");
		}
	}

	void End()
	{
		count = 0;
		if(ptr != nullptr)
			free(ptr);
	}

	List(T* ptr, uint count):
		ptr(ptr),
		count(count)
	{}

	List() :
		ptr((T*)malloc(0)),
		count(0)
	{}
};

class Shader
{
public:
	uint program = 0u;

	void Use()
	{
		glUseProgram(program);
	}

	void End()
	{
		glDeleteProgram(program);
	}

	Shader():
		program(0)
	{}

	Shader(char* vertexShaderSource, char* fragmentShaderSource)
	{
		uint vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);

		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		int  success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
		}

		uint fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
		}

		program = glCreateProgram();

		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	
	static Shader OpenShader(char* vertexShaderFilename, char* fragmentShaderFilename)
	{
		char* vertexShader;
		vertexShader = vertexShaderFilename;
		char* fragmentShader;
		fragmentShader = fragmentShaderFilename;
		return Shader(vertexShader, fragmentShader);
	}
};

#pragma region Shader uniform variable types

struct Shadf
{
	char* location;
	float* value;

	Shadf(char* location, float* value) :
		location(location),
		value(value)
	{}
};

struct Shadi
{
	char* location;
	int* value;

	Shadi(char* location, int* value) :
		location(location),
		value(value)
	{}
};

struct Shadu
{
	char* location;
	uint* value;

	Shadu(char* location, uint* value) :
		location(location),
		value(value)
	{}
};

#pragma endregion

class ShaderInstance
{
public:

#pragma region uniform variable lists

	List<Shadf> shad1f;
	List<Shadf> shad2f;
	List<Shadf> shad3f;
	List<Shadf> shad4f;

	List<Shadi> shad1i;
	List<Shadi> shad2i;
	List<Shadi> shad3i;
	List<Shadi> shad4i;

	List<Shadu> shad1u;
	List<Shadu> shad2u;
	List<Shadu> shad3u;
	List<Shadu> shad4u;

#pragma endregion

	Shader shader;
	void Use()
	{
		shader.Use();

		for (uint i = 0; i < shad1f.count; i++)
			glUniform1f(glGetUniformLocation(shader.program, shad1f.ptr[i].location), shad1f.ptr[i].value[0]);

		for (uint i = 0; i < shad2f.count; i++)
			glUniform2f(glGetUniformLocation(shader.program, shad2f.ptr[i].location), shad2f.ptr[i].value[0], shad2f.ptr[i].value[1]);

		for (uint i = 0; i < shad3f.count; i++)
			glUniform3f(glGetUniformLocation(shader.program, shad3f.ptr[i].location), shad3f.ptr[i].value[0], shad3f.ptr[i].value[1], shad3f.ptr[i].value[2]);

		for (uint i = 0; i < shad4f.count; i++)
		{
			glUniform4f(glGetUniformLocation(shader.program, shad4f.ptr[i].location), shad4f.ptr[i].value[0], shad4f.ptr[i].value[1], shad4f.ptr[i].value[2], shad4f.ptr[i].value[3]);
		}

		for (uint i = 0; i < shad1i.count; i++)
			glUniform1i(glGetUniformLocation(shader.program, shad1i.ptr[i].location), shad1i.ptr[i].value[0]);

		for (uint i = 0; i < shad2i.count; i++)
			glUniform2i(glGetUniformLocation(shader.program, shad2i.ptr[i].location), shad2i.ptr[i].value[0], shad2i.ptr[i].value[1]);

		for (uint i = 0; i < shad3f.count; i++)
			glUniform3i(glGetUniformLocation(shader.program, shad3i.ptr[i].location), shad3i.ptr[i].value[0], shad3i.ptr[i].value[1], shad3i.ptr[i].value[2]);

		for (uint i = 0; i < shad4i.count; i++)
			glUniform4i(glGetUniformLocation(shader.program, shad4i.ptr[i].location), shad4i.ptr[i].value[0], shad4i.ptr[i].value[1], shad4i.ptr[i].value[2], shad4i.ptr[i].value[3]);


		for (uint i = 0; i < shad1u.count; i++)
			glUniform1ui(glGetUniformLocation(shader.program, shad1u.ptr[i].location), shad1u.ptr[i].value[0]);

		for (uint i = 0; i < shad2u.count; i++)
			glUniform2ui(glGetUniformLocation(shader.program, shad2u.ptr[i].location), shad2u.ptr[i].value[0], shad2u.ptr[i].value[1]);

		for (uint i = 0; i < shad3u.count; i++)
			glUniform3ui(glGetUniformLocation(shader.program, shad3u.ptr[i].location), shad3u.ptr[i].value[0], shad3u.ptr[i].value[1], shad3u.ptr[i].value[2]);

		for (uint i = 0; i < shad4u.count; i++)
			glUniform4ui(glGetUniformLocation(shader.program, shad4u.ptr[i].location), shad4u.ptr[i].value[0], shad4u.ptr[i].value[1], shad4u.ptr[i].value[2], shad4u.ptr[i].value[3]);
	}

	void End()
	{
		shader.End();
	}

	ShaderInstance(const Shader& shader) :
		shader{ shader },
#pragma region shader uniform init

		shad1f(List<Shadf>()),
		shad2f(List<Shadf>()),
		shad3f(List<Shadf>()),
		shad4f(List<Shadf>()),

		shad1i(List<Shadi>()),
		shad2i(List<Shadi>()),
		shad3i(List<Shadi>()),
		shad4i(List<Shadi>()),

		shad1u(List<Shadu>()),
		shad2u(List<Shadu>()),
		shad3u(List<Shadu>()),
		shad4u(List<Shadu>())

#pragma endregion
	{ }

	ShaderInstance() :
		ShaderInstance(Shader())
	{}
};