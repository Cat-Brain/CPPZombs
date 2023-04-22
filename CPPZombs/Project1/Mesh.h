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

Mesh quad, screenSpaceQuad, line, dot, rightTriangle, lineThick, cube;

vector<Mesh*> meshes{ &quad, &screenSpaceQuad, &line, &dot, &lineThick }; // Destroys them at the end.