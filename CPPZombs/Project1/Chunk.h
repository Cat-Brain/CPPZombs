#include "LightSource.h"

#define RUBY_SOIL_MULTIPLIER 2.f
enum class TILE : byte // The " : byte" forces this enum to be a single byte in size instead of the default of 4.
{
	AIR, ROCK, SAND, BAD_SOIL, MID_SOIL, MAX_SOIL, RUBY_SOIL, HIGH_ROCK, SNOW
};

RGBA tileColors[] = { RGBA(0, 0, 0, 0), RGBA(61, 79, 72), RGBA(212, 199, 89), RGBA(143, 111, 79), RGBA(102, 64, 27), RGBA(98, 102, 27),
RGBA(166, 45, 87), RGBA(127, 140, 94), RGBA(219, 211, 237) };

int tileHealths[] = { 0, 60, 40, 40, 40, 40, 120, 40, 10 };

uint chunkTexture = 0;
unique_ptr<byte[]> chunkColorData;

struct TileData
{
	int chunkIndex;
	byte x, y, z;
	int damageDealt;
	float lastDamage;

	TileData(int chunkIndex, byte x, byte y, byte z, int damageDealt = 0) :
		chunkIndex(chunkIndex), x(x), y(y), z(z), damageDealt(damageDealt), lastDamage(tTime) { }

	bool Damage(int damage);
};

vector<TileData> tileDatas;
constexpr uint quadInd[] = {0, 1, 2, 0, 2, 3};

class Chunk : public vector<int>
{
public:
	iVec3 pos;
	byte tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
	Mesh mesh;
	uint vbo = 0, ebo = 0, vao = 0, indCount = 0; // For mesh rendering.
	int xPlus, xMin, yPlus, yMin, zPlus, zMin;

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
	void Finalize();

	bool Overlaps(Vec2 pos, Vec2 dimensions)
	{
		return pos.x - dimensions.x >= this->pos.x + dimensions.x && pos.x < this->pos.x + CHUNK_WIDTH &&
			pos.y - dimensions.y >= this->pos.y + dimensions.y && pos.y < this->pos.y + CHUNK_WIDTH;
	}

	static bool DamageTile(int damage, int index, int x, int y, int z)
	{
		for (int i = 0; i < tileDatas.size(); i++)
			if (tileDatas[i].chunkIndex == index && tileDatas[i].x == x && tileDatas[i].y == y && tileDatas[i].z == z)
			{
				if (tileDatas[i].Damage(damage))
				{
					tileDatas.erase(tileDatas.begin() + i);
					return true;
				}
				return false;
			}

		TileData data = TileData(index, x, y, z);
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
		if (indCount == 0) return;
		Vec3 drawPos = Vec3(pos) - game->PlayerPos() - game->screenOffset;
		glUniform3f(glGetUniformLocation(chunkShader, "position"), drawPos.x, drawPos.y, drawPos.z);
		glUniform3f(glGetUniformLocation(chunkShader, "globalPosition"), pos.x, pos.y, pos.z);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indCount), GL_UNSIGNED_INT, 0);
	}

	byte TileAtCPos(iVec3 pos)
	{
		return tiles[pos.x][pos.y][pos.z];
	}

	byte TileAtPos(iVec3 pos)
	{
		return tiles[pos.x - this->pos.x][pos.y - this->pos.y][pos.z];
	}

	void SetTileAtPos(iVec3 pos, byte newValue)
	{
		if (newValue == tiles[pos.x - this->pos.x][pos.y - this->pos.y][pos.z - this->pos.z]) return;
		tiles[pos.x - this->pos.x][pos.y - this->pos.y][pos.z - this->pos.z] = newValue;
		RegenerateMesh();
	}

	void DestroyMesh()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
		glDeleteVertexArrays(1, &vao);
	}

	void GenerateVert(vector<float>& data, size_t posStart, byte x, byte y, byte z, float nX, float nY, float nZ, JRGB col)
	{
		data[posStart] = x;
		data[posStart + 1] = y;
		data[posStart + 2] = z;

		data[posStart + 3] = col.r / 255.f; data[posStart + 4] = col.g / 255.f; data[posStart + 5] = col.b / 255.f;
		data[posStart + 6] = nX; data[posStart + 7] = nY; data[posStart + 8] = nZ;
	}

	// Add genDir[1] stuff!
	void GenTile(vector<float>& data, byte x, byte y, byte z, bool genZ, bool genNZ, bool genY, bool genNY, bool genX, bool genNX)
	{
		if (tiles[x][y][z] == 0)
			return;
		byte sideCount = int(genZ) + int(genY) + int(genNY) + int(genX) + int(genNX);
		data.resize(data.size() + size_t(sideCount) * 36);
		int r = tileColors[tiles[x][y][z]].r, g = tileColors[tiles[x][y][z]].g, b = tileColors[tiles[x][y][z]].b;
		if (genZ)
		{
			int posStart = int(data.size()) - sideCount * 36;
			GenerateVert(data, posStart, x, y, z + 1, 0, 0, 1, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x, y + 1, z + 1, 0, 0, 1, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y + 1, z + 1, 0, 0, 1, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y, z + 1, 0, 0, 1, JRGB(r, g, b));
			sideCount--;
		}
		if (genY)
		{
			int posStart = int(data.size()) - sideCount * 36;
			GenerateVert(data, posStart, x, y + 1, z, 0, 1, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x, y + 1, z + 1, 0, 1, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y + 1, z + 1, 0, 1, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y + 1, z, 0, 1, 0, JRGB(r, g, b));
			sideCount--;
		}
		if (genNY)
		{
			int posStart = int(data.size()) - sideCount * 36;
			GenerateVert(data, posStart, x, y, z, 0, -1, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x, y, z + 1, 0, -1, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y, z + 1, 0, -1, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y, z, 0, -1, 0, JRGB(r, g, b));
			sideCount--;
		}
		if (genX)
		{
			int posStart = int(data.size()) - sideCount * 36;
			GenerateVert(data, posStart, x + 1, y + 1, z, 1, 0, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y + 1, z + 1, 1, 0, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y, z + 1, 1, 0, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x + 1, y, z, 1, 0, 0, JRGB(r, g, b));
			sideCount--;
		}
		if (genNX)
		{
			int posStart = int(data.size()) - sideCount * 36;
			GenerateVert(data, posStart, x, y + 1, z, -1, 0, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x, y + 1, z + 1, -1, 0, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x, y, z + 1, -1, 0, 0, JRGB(r, g, b));
			posStart += 9;
			GenerateVert(data, posStart, x, y, z, -1, 0, 0, JRGB(r, g, b));
		}
	}

	void GenerateMesh();

	void RegenerateMesh()
	{
		DestroyMesh();
		GenerateMesh();
	}
};