#include "LightSource.h"

#define RUBY_SOIL_MULTIPLIER 2.f
enum class TILE : byte // The " : byte" forces this enum to be a single byte in size instead of the default of 4.
{
	ROCK, SAND, BAD_SOIL, MID_SOIL, MAX_SOIL, RUBY_SOIL
};

RGBA tileColors[] = { RGBA(61, 79, 72), RGBA(212, 199, 89), RGBA(143, 111, 79), RGBA(102, 64, 27), RGBA(98, 102, 27), RGBA(166, 45, 87)};

uint chunkTexture = 0;
unique_ptr<byte[]> chunkColorData;
class Chunk : public vector<int>
{
public:
	iVec2 pos;
	byte tiles[CHUNK_WIDTH][CHUNK_WIDTH];

	static void Init()
	{
		chunkColorData = make_unique<byte[]>(CHUNK_WIDTH * CHUNK_WIDTH * 4);
		glGenTextures(1, &chunkTexture);
		glBindTexture(GL_TEXTURE_2D, chunkTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CHUNK_WIDTH, CHUNK_WIDTH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	Chunk(iVec2 pos = vZero) :
		vector{}, pos(pos)
	{
		for (int x = 0; x < CHUNK_WIDTH; x++)
			for (int y = 0; y < CHUNK_WIDTH; y++)
			{
				float noise = game->planet->worldNoise.GetNoise(float(pos.x + x), float(pos.y + y));
				tiles[x][y] = UnEnum(noise < 0 ? TILE::ROCK : noise < 0.5f ? TILE::SAND : noise < 0.75f ?
					TILE::BAD_SOIL : noise < 0.875f ? TILE::MID_SOIL : TILE::MAX_SOIL);
			}
	}

	bool Overlaps(Vec2 pos, Vec2 dimensions)
	{
		return pos.x - dimensions.x >= this->pos.x + dimensions.x && pos.x < this->pos.x + CHUNK_WIDTH &&
			pos.y - dimensions.y >= this->pos.y + dimensions.y && pos.y < this->pos.y + CHUNK_WIDTH;
	}

	static iVec2 ToSpace(Vec2 pos)
	{
		return ToIV2(pos / float(CHUNK_WIDTH));
	}

	static std::pair<iVec2, iVec2> MinMaxPos(Vec2 pos, float radius)
	{
		return { ToSpace(pos - radius), ToSpace((pos + radius)) };
	}

	void Draw()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, chunkTexture);

			for (int y = 0, i = 0; y < CHUNK_WIDTH; y++)
				for (int x = 0; x < CHUNK_WIDTH; x++)
				{
					chunkColorData[i * 4] = tileColors[tiles[x][y]].r;
					chunkColorData[i * 4 + 1] = tileColors[tiles[x][y]].g;
					chunkColorData[i * 4 + 2] = tileColors[tiles[x][y]].b;
					chunkColorData[i * 4 + 3] = 1.f;
					i++;
				}
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CHUNK_WIDTH, CHUNK_WIDTH, GL_RGBA, GL_UNSIGNED_BYTE, chunkColorData.get());

		game->DrawChunk(pos, Vec2(CHUNK_WIDTH));
	}

	byte TileAtPos(iVec2 pos)
	{
		return tiles[pos.x - this->pos.x][pos.y - this->pos.y];
	}

	void SetTileAtPos(iVec2 pos, byte newValue)
	{
		tiles[pos.x - this->pos.x][pos.y - this->pos.y] = newValue;
	}
};