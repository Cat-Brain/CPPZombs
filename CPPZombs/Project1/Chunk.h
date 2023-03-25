#include "LightSource.h"

enum class TILE : byte // The " : byte" forces this enum to be a single byte in size instead of the default of 4.
{
	SAND, ROCK, BAD_SOIL, MID_SOIL, MAX_SOIL
};

class Chunk : public vector<int>
{
public:
	iVec2 pos;
	byte tiles[CHUNK_WIDTH][CHUNK_WIDTH];

	Chunk(iVec2 pos = vZero) :
		vector{}, pos(pos)
	{
		for (int x = 0; x < CHUNK_WIDTH; x++)
			for (int y = 0; y < CHUNK_WIDTH; y++)
			{
				float noise = game->planet->worldNoise.GetNoise(float(pos.x + x), float(pos.y + y));
				tiles[x][y] = UnEnum(noise > 0.5f ? TILE::ROCK : TILE::SAND);
			}
	}

	bool Overlaps(Vec2 pos, Vec2 dimensions)
	{
		return pos.x - dimensions.x >= this->pos.x + dimensions.x && pos.x < this->pos.x + CHUNK_WIDTH &&
			pos.y - dimensions.y >= this->pos.y + dimensions.y && pos.y < this->pos.y + CHUNK_WIDTH;
	}

	static iVec2 ToSpace(Vec2 pos)
	{
		return iVec2(pos.x / CHUNK_WIDTH, pos.y / CHUNK_WIDTH);
	}

	static std::pair<iVec2, iVec2> MinMaxPos(Vec2 pos, float radius)
	{
		return { ToSpace(pos - radius), ToSpace((pos + radius)) };
	}

	void Draw()
	{

	}
};