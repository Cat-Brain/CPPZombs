#include "LightSource.h"

#define RUBY_SOIL_MULTIPLIER 2.f
enum class TILE : byte // The " : byte" forces this enum to be a single byte in size instead of the default of 4.
{
	AIR, ROCK, SAND, BAD_SOIL, MID_SOIL, MAX_SOIL, RUBY_SOIL, HIGH_ROCK, SNOW
};

RGBA tileColors[] = { RGBA(0, 0, 0, 0), RGBA(61, 79, 72), RGBA(212, 199, 89), RGBA(143, 111, 79), RGBA(102, 64, 27), RGBA(98, 102, 27),
RGBA(166, 45, 87), RGBA(127, 140, 94), RGBA(219, 211, 237) };

int tileHealths[] = { 0, 4, 1, 2, 2, 2, 4, 2, 1 };

uint chunkTexture = 0;
unique_ptr<byte[]> chunkColorData;

struct TileData
{
	int chunkIndex;
	byte x, y;
	int damageDealt;
	float lastDamage;

	TileData(int chunkIndex, byte x, byte y, int damageDealt = 0) :
		chunkIndex(chunkIndex), x(x), y(y), damageDealt(damageDealt), lastDamage(tTime) { }

	bool Damage(int damage);
};

vector<TileData> tileDatas;

class Chunk : public vector<int>
{
public:
	iVec3 pos;
	byte tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];

	static void Init()
	{
		tileDatas = vector<TileData>();

		chunkColorData = make_unique<byte[]>(CHUNK_WIDTH * CHUNK_WIDTH * 4);
		glGenTextures(1, &chunkTexture);
		glBindTexture(GL_TEXTURE_2D, chunkTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHUNK_WIDTH, CHUNK_WIDTH, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	Chunk(iVec3 pos = vZero);

	bool Overlaps(Vec2 pos, Vec2 dimensions)
	{
		return pos.x - dimensions.x >= this->pos.x + dimensions.x && pos.x < this->pos.x + CHUNK_WIDTH &&
			pos.y - dimensions.y >= this->pos.y + dimensions.y && pos.y < this->pos.y + CHUNK_WIDTH;
	}

	static bool DamageTile(int damage, int index, int x, int y)
	{
		for (int i = 0; i < tileDatas.size(); i++)
			if (tileDatas[i].chunkIndex == index && tileDatas[i].x == x && tileDatas[i].y == y)
			{
				if (tileDatas[i].Damage(damage))
				{
					tileDatas.erase(tileDatas.begin() + i);
					return true;
				}
				return false;
		}

		TileData data = TileData(index, x, y);
		if (!data.Damage(damage))
		{
			tileDatas.push_back(data);
			return false;
		}
		return true;
	}

	static iVec3 ToSpace(Vec3 pos)
	{
		return ToIV3(pos / float(CHUNK_WIDTH));
	}

	static std::pair<iVec3, iVec3> MinMaxPos(Vec3 pos, float radius)
	{
		return { ToSpace(pos - radius), ToSpace((pos + radius)) };
	}

	void Draw()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, chunkTexture);
		for (int z = 0; z < 2/*CHUNK_WIDTH*/; z++)
		{
			for (int y = 0, i = 0; y < CHUNK_WIDTH; y++)
				for (int x = 0; x < CHUNK_WIDTH; x++)
				{
					chunkColorData[size_t(i) * 4] = tileColors[tiles[x][y][z]].r;
					chunkColorData[size_t(i) * 4 + 1] = tileColors[tiles[x][y][z]].g;
					chunkColorData[size_t(i) * 4 + 2] = tileColors[tiles[x][y][z]].b;
					chunkColorData[size_t(i) * 4 + 3] = tileColors[tiles[x][y][z]].a;
					i++;
				}
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CHUNK_WIDTH, CHUNK_WIDTH, GL_RGBA, GL_UNSIGNED_BYTE, chunkColorData.get());

			game->DrawChunk(pos, Vec2(CHUNK_WIDTH));
		}
	}

	byte TileAtCPos(iVec3 pos)
	{
		return tiles[pos.x][pos.y][pos.z];
	}

	byte TileAtPos(iVec2 pos)
	{
		return tiles[pos.x - this->pos.x][pos.y - this->pos.y][0];
	}

	void SetTileAtPos(iVec2 pos, byte newValue)
	{
		tiles[pos.x - this->pos.x][pos.y - this->pos.y][0] = newValue;
	}
};