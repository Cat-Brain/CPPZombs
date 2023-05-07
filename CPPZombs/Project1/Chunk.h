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

	void GenerateMesh()
	{
		bool allEmpty = true;
		for (byte x = 0; x < CHUNK_WIDTH; x++)
			for (byte y = 0; y < CHUNK_WIDTH; y++)
				for (byte z = 0; z < CHUNK_WIDTH; z++)
					allEmpty &= tiles[x][y][z] == 0;
		if (allEmpty)
		{
			indCount = 0;
			return;
		}
		vector<float> data{}; // Goes pos.x, pos.y, pos.z, color.r, color.g, color.b, normal.x, normal.y, normal.z
		for (byte x = 0; x < CHUNK_WIDTH; x++)
			for (byte y = 0; y < CHUNK_WIDTH; y++)
				for (byte z = 0; z < CHUNK_WIDTH; z++)
				{
					if (tiles[x][y][z] == 0)
						continue;
					bool genFor = z >= CHUNK_WIDTH - 1 || tiles[x][y][z + 1] == 0,
						genUp = y >= CHUNK_WIDTH - 1 || tiles[x][y + 1][z] == 0,
						genDown = y == 0 || tiles[x][y - 1][z] == 0,
						genRight = x >= CHUNK_WIDTH - 1 || tiles[x + 1][y][z] == 0, 
						genLeft = x == 0 || tiles[x - 1][y][z] == 0;
					byte sideCount = int(genFor) + int(genUp) + int(genDown) + int(genRight) + int(genLeft);
					data.resize(data.size() + size_t(sideCount) * 36);
					int r = tileColors[tiles[x][y][z]].r, g = tileColors[tiles[x][y][z]].g, b = tileColors[tiles[x][y][z]].b;
					if (genFor)
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
					if (genUp)
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
					if (genDown)
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
					if (genRight)
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
					if (genLeft)
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

		vector<uint> indices = vector<uint>(data.size() / 4 * 6);
		for (uint i = 0; i < indices.size(); i++)
			indices[i] = quadInd[i % 6] + i / 6 * 4;


		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), static_cast<void*>(data.data()), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), static_cast<void*>(indices.data()), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);

		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));

		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

		glEnableVertexAttribArray(2);

		indCount = static_cast<uint>(indices.size());
	}

	void RegenerateMesh()
	{
		DestroyMesh();
		GenerateMesh();
	}
};