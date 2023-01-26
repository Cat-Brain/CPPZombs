#include "JRGB.h"

class Mesh
{
public:
	uint vbo, ebo, vao;
	vector<Vec2> vertices;

	Mesh(vector<Vec2> vertices) : vertices(vertices)
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 2, &vertices[0], GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void Draw()
	{
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}
};