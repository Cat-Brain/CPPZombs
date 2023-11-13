#include "Resources.h"

class Texture
{
public:
	uint id, width, height, spriteCount;

	Texture() : id(0), width(0), height(0), spriteCount(0) { }

	Texture(int location, int type = PNG_FILE, uint spriteCount = 1) : spriteCount(spriteCount)
	{
		Resource resource = Resource(location, type);
		if (resource.size == 0)
			std::cout << "Failed to load texture!\n";
		vector<unsigned char> loadedData;
		unsigned long width, height;
		int error;
		if ((error = decodePNG(loadedData, width, height, static_cast<const unsigned char*>(resource.ptr), resource.size)))
			std::cout << "Error decrypting texture! Error code = " << error << "\n";
		if (loadedData.size() == 0)
			std::cout << "Failed to decrypt texture!\n";
		this->width = width;
		this->height = height;

		// OpenGL part of texture loading.
		glGenTextures(1, &id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id);
		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// load and generate the texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<void*>(loadedData.data()));
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void Activate(GLenum position = GL_TEXTURE0)
	{
		glActiveTexture(position);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Destroy()
	{
		glDeleteTextures(1, &id);
	}
};

Texture spriteSheet, reticleSprite, stippleTexture;
std::tuple<uint, vector<std::tuple<Texture&, int, int>>> textures{ PNG_FILE, {{spriteSheet, SPRITE_SHEET, 7}, {reticleSprite, RETICLE_SPRITE, 1},
	{stippleTexture, STIPPLE_TEXTURE, 1}} };