#include "JRGB.h"

class Mesh
{
public:
	uint vbo, ebo, vao;

	Mesh(vector<Vec2> vertices)
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 2, &vertices[0], GL_STATIC_DRAW);
	}
};