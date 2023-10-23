#include "JRGB.h"

class Mesh
{
public:
	uint vbo, ebo, vao;
	vector<float> vertices;
	vector<uint> indices;
	GLenum mode;

	Mesh() : vbo(0), ebo(0), vao(0), vertices{}, indices{}, mode(GL_TRIANGLES) { }

	Mesh(vector<float> vertices, vector<uint> indices, GLenum mode = GL_TRIANGLES, GLenum drawMode = GL_STATIC_DRAW, uint dimension = 2) :
		vertices(vertices), indices(indices), mode(mode)
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], drawMode);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], drawMode);

		glVertexAttribPointer(0, dimension, GL_FLOAT, GL_FALSE, dimension * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void Draw()
	{
		glBindVertexArray(vao);
		glDrawElements(mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	}

	void Destroy()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
		glDeleteVertexArrays(1, &vao);
	}
};

class Mesh2
{
public:
	uint vbo, vao;
	vector<float> data;
	GLenum mode;

	Mesh2() : vbo(0), vao(0), data{}, mode(GL_TRIANGLES) { }

	Mesh2(vector<float> data, GLenum mode = GL_TRIANGLES, GLenum drawMode = GL_STATIC_DRAW, uint dimension = 3) :
		data(data), mode(mode)
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], drawMode);

		glVertexAttribPointer(0, dimension, GL_FLOAT, GL_FALSE, dimension * sizeof(float) * 2, (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, dimension, GL_FLOAT, GL_FALSE, dimension * sizeof(float) * 2, (void*)(sizeof(float) * dimension));
		glEnableVertexAttribArray(1);
	}

	void Draw()
	{
		glBindVertexArray(vao);
		glDrawArrays(mode, 0, data.size());
	}

	void Destroy()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}
};

Mesh quad, screenSpaceQuad, line, dot, rightTriangle, lineThick, cube;
Mesh2 shrub;

vector<Mesh*> meshes{ &quad, &screenSpaceQuad, &line, &dot, &lineThick, &cube, // Destroys them at the end.
	};
vector<Mesh2*> mesh2s{ &shrub }; // Destroys them at the end.