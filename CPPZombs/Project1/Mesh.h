#include "JRGB.h"

class Mesh
{
public:
	uint vbo, ebo, vao;
	vector<float> vertices;
	vector<uint> indices;
	GLenum mode;

	Mesh() : vbo(0), ebo(0), vao(0), vertices{}, indices{} { }

	Mesh(vector<float> vertices, vector<uint> indices, GLenum mode = GL_TRIANGLES) :
		vertices(vertices), indices(indices), mode(mode)
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
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

Mesh quad, screenSpaceQuad, line;

vector<Mesh*> meshes{ &quad, &screenSpaceQuad, &line }; // Destroys them at the end.