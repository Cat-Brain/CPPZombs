#include "ObjectWorld.h"

class Transform : public Component
{
public:
	vec3 position;
	vec3 rotation;
	vec3 scale;

	void Print()
	{
		printf("p(%f, %f, %f), r(%f, %f, %f), s(%f, %f, %f)\n",
			position[0], position[1], position[2],
			rotation[0], rotation[1], rotation[2],
			scale[0], scale[1], scale[2]);
	}

	void Print2()
	{
		printf("p(%f, %f, %f), r(%f, %f, %f), s(%f, %f, %f)",
			position[0], position[1], position[2],
			rotation[0], rotation[1], rotation[2],
			scale[0], scale[1], scale[2]);
	}

	Transform(void* object) :
		Component(object),
		position{ 0.0f, 0.0f, 0.0f },
		rotation{ 0.0f, 0.0f, 0.0f },
		scale{ 1.0f, 1.0f, 1.0f }
	{ }

	Transform(void* object, vec3 position, vec3 rotation, vec3 scale):
		Component(object),
		position{ position[0], position[1], position[2] },
		rotation{ rotation[0], rotation[1], rotation[2] },
		scale{ scale[0], scale[1], scale[2] }
	{ }
};

class Mesh : public Component
{
protected:
	uint VBO, VAO;
	ShaderInstance* shader;

public:
	float* vertices;
	uint vertCount;

	void Start()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((unsigned long long)vertCount * (unsigned long long)3u * sizeof(float)), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void RenderUpdate(float deltaTime)
	{
		shader->Use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertCount);
	}

	void End()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	Mesh(void* object, float* vertices, uint vertCount, ShaderInstance* shader) :
		Component(object),
		vertices(vertices),
		vertCount(vertCount),
		shader(shader),
		VBO(0),
		VAO(0)
	{ }
};

class Mesh2 : public Mesh
{
protected:
	uint EBO;

public:
	uint* triangles;
	uint triangleCount;

	void Start()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, (unsigned long long)12 * (unsigned long long)3u * sizeof(float), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned long long)triangleCount * (unsigned long long)3u * sizeof(uint), triangles, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void RenderUpdate(float deltaTime)
	{
		shader->Use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, triangleCount * 3, GL_UNSIGNED_INT, 0);
	}

	void End()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	Mesh2(void* object, float* vertices, uint vertCount, uint* triangles, uint triangleCount, ShaderInstance* shader):
		Mesh(object, vertices, vertCount, shader),
		triangles(triangles),
		triangleCount(triangleCount),
		EBO(0)
	{ }
};