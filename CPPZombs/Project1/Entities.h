#include "Chunk.h"
struct EntityIndex // For sorting.
{
	int index, valueForSorting;

	EntityIndex(int index = 0, int valueForSorting = 0) : index(index), valueForSorting(valueForSorting) {}

	bool operator < (const EntityIndex& other) const
	{
		return (valueForSorting < other.valueForSorting);
	}
};

struct RaycastHit
{
	Vec3 pos, norm;
	float dist;
	bool chunkOrEntity; // false = chunk, true = entity
	int index;

	RaycastHit(Vec3 pos = vZero, Vec3 norm = vZero, float dist = std::numeric_limits<float>::infinity(), bool chunkOrEntity = false, int index = -1) :
		pos(pos), norm(norm), dist(dist), chunkOrEntity(chunkOrEntity), index(index) { }
};

class LightSource;
class Entities : public vector<unique_ptr<Entity>>
{
#pragma region Control Stuff
protected:
	int index;

public:
	bool addedEntity;
	vector<Entity*> sortedNCEntities; // The NC stands for Non-Collectible.
	vector<Entity*> collectibles; // sortedNCEntities and collectibles are the most accurate, the others are less so.
	vector<unique_ptr<LightSource>> lightSources;
	vector<unique_ptr<LightSource>> darkSources;
	vector<unique_ptr<Particle>> particles;
	vector<Entity*> toDelEntities;
	vector<unique_ptr<Entity>> toAddEntities;
	vector<Chunk> chunks;
	vector<Chunk*> renderedChunks;
	bool createdChunkThisFrame = false;

	Entities() :
		vector(0), addedEntity(false), index(0)
	{
		Chunk::Init();
	}

	~Entities()
	{
		for (Chunk chunk : chunks)
			chunk.DestroyMesh();
	}

	void DelayedDestroy(Entity* entity)
	{
		toDelEntities.push_back(entity);
	}

	void DelayedPushBack(unique_ptr<Entity> entity)
	{
		toAddEntities.push_back(std::move(entity));
	}

	void push_back(unique_ptr<Entity> entity)
	{
		Entity* entityPtr = entity.get();

		addedEntity = true;
		if (entityPtr->isCollectible)
			collectibles.push_back(entityPtr);
		else
		{
			index++;
			sortedNCEntities.insert(sortedNCEntities.begin(), entityPtr);
		}
		int index = -1;
		for (int i = 0; i < size(); i++)
			if (!(*this)[i])
			{
				index = i;
				(*this)[index] = std::move(entity);
				break;
			}

		if (index == -1)
		{
			index = static_cast<int>(size());
			vector<unique_ptr<Entity>>::push_back(std::move(entity));
		}

		vector<int> chunkOverlaps = FindCreateChunkOverlaps(entityPtr->pos, entityPtr->radius);
		for (int chunk : chunkOverlaps)
			chunks[chunk].push_back(index);
	}
#pragma endregion
#pragma region Overlaps and Collision Stuff
#pragma region Chunk Stuff
	int ChunkAtPos(iVec3 pos)
	{
		for (int i = 0; i < chunks.size(); i++)
			if (chunks[i].pos == pos)
				return i;
		return -1;
	}

	Chunk* ChunkAtFPos(Vec3 pos)
	{
		return &chunks[ChunkAtPos(ToIV3(pos * (1.f / CHUNK_WIDTH)) * CHUNK_WIDTH)];
	}

	vector<int> MainChunkOverlaps(iVec3 minPos, iVec3 maxPos) // In chunk coords, do NOT plug in normal space coords.
	{
		vector<int> result((maxPos.x - minPos.x + 1) * (maxPos.y - minPos.y + 1) * (maxPos.z - minPos.z + 1));
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				for (int z = minPos.z; z <= maxPos.z; z++)
					result[i++] = ChunkAtPos(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, z * CHUNK_WIDTH));
		return result;
	}

	vector<int> FindCreateChunkOverlapsMain(iVec3 minPos, iVec3 maxPos) // A safer version of MainChunksOveraps.
	{
		vector<int> result((maxPos.x - minPos.x + 1) * (maxPos.y - minPos.y + 1) * (maxPos.z - minPos.z + 1));
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				for (int z = minPos.z; z <= maxPos.z; z++)
				{
					int chunk = ChunkAtPos(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, z * CHUNK_WIDTH));
					if (chunk != -1)
					{
						result[i++] = chunk;
						continue;
					}
					result[i++] = static_cast<int>(chunks.size());
					chunks.push_back(Chunk(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, z * CHUNK_WIDTH)));
					chunks[chunks.size() - 1].Finalize();
				}
		return result;
	}

	vector<int> ChunkOverlaps(Vec3 pos, float radius)
	{
		std::pair<iVec3, iVec3> minMaxPos = Chunk::MinMaxPos(pos, radius);
		return MainChunkOverlaps(minMaxPos.first, minMaxPos.second);
	}

	vector<int> FindCreateChunkOverlaps(Vec3 pos, float radius)
	{
		std::pair<iVec3, iVec3> minMaxPos = Chunk::MinMaxPos(pos, radius);
		return FindCreateChunkOverlapsMain(minMaxPos.first, minMaxPos.second);
	}

	void RemoveFromChunksMain(int index, iVec3 minPos, iVec3 maxPos)
	{
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				for (int z = minPos.z; z <= maxPos.z; z++)
				{
					Chunk& chunk = chunks[ChunkAtPos(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, z * CHUNK_WIDTH))];
					chunk.erase(find(chunk.begin(), chunk.end(), index));
				}
	}

	void RemoveFromChunks(int index, Vec3 pos, float radius)
	{
		std::pair<iVec3, iVec3> minMaxPos = Chunk::MinMaxPos(pos, radius);
		RemoveFromChunksMain(index, minMaxPos.first, minMaxPos.second);
	}
#pragma endregion
#pragma region Overlap Stuff
	bool OverlapsTile(Vec3 pos, float radius)
	{
		vector<int> chunkOverlaps = FindCreateChunkOverlaps(pos, radius);
		for (int i : chunkOverlaps)
		{
			iVec3 minPos = glm::max(vZeroI, ToIV3(pos - radius) - chunks[i].pos);
			iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - chunks[i].pos);
			for (int x = minPos.x; x <= maxPos.x; x++)
				for (int y = minPos.y; y <= maxPos.y; y++)
					for (int z = minPos.z; z <= maxPos.z; z++)
						if (chunks[i].TileAtCPos(Vec3(x, y, z)) != UnEnum(TILE::AIR) && BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, z) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
							return true;
		}
		return false;
	}

	bool CubeDoesOverlap(vector<int> chunkOverlaps, Vec3 pos, Vec3 dim, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter] && BoxCircleOverlap((*this)[*iter]->pos, (*this)[*iter]->radius, pos, dim) && (*this)[*iter]->active && func(from, (*this)[*iter].get())) return true;
		return false;
	}

	inline bool CubeDoesOverlap(iVec3 minPos, iVec3 maxPos, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		return CubeDoesOverlap(FindCreateChunkOverlapsMain(Chunk::ToSpace(minPos), Chunk::ToSpace(maxPos)),
			Vec3(maxPos + minPos + 1) * 0.5f, Vec3(maxPos - minPos + 1) * 0.5f, func, from);
	}

	bool DoesOverlap(vector<int> chunkOverlaps, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter] && (*this)[*iter]->Overlaps(pos, radius) && (*this)[*iter]->active && func(from, (*this)[*iter].get())) return true;
		return false;
	}

	inline bool DoesOverlap(Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		return DoesOverlap(FindCreateChunkOverlaps(pos, radius), pos, radius, func, from);
	}

	Entity* FirstOverlap(vector<int> chunkOverlaps, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter] && (*this)[*iter]->Overlaps(pos, radius) && (*this)[*iter]->active && func(from, (*this)[*iter].get())) return (*this)[*iter].get();
		return nullptr;
	}

	inline Entity* FirstOverlap(Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		return FirstOverlap(FindCreateChunkOverlaps(pos, radius), pos, radius, func, from);
	}

	vector<Entity*> FindOverlaps(vector<int> chunkOverlaps, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		vector<Entity*> overlaps{};
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter] && (*this)[*iter]->Overlaps(pos, radius) && (*this)[*iter]->active && func(from, (*this)[*iter].get()) &&
					find(overlaps.begin(), overlaps.end(), (*this)[*iter].get()) == overlaps.end()) overlaps.push_back((*this)[*iter].get());
		return overlaps;
	}

	inline vector<Entity*> FindOverlaps(Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		return FindOverlaps(FindCreateChunkOverlaps(pos, radius), pos, radius, func, from);
	}

	vector<Entity*> FindAllOverlaps(vector<int> chunkOverlaps, Vec3 pos, float radius)
	{
		vector<Entity*> overlaps{};
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter] && (*this)[*iter]->Overlaps(pos, radius) && (*this)[*iter]->active &&
					find(overlaps.begin(), overlaps.end(), (*this)[*iter].get()) == overlaps.end()) overlaps.push_back((*this)[*iter].get());
		return overlaps;
	}

	inline vector<Entity*> FindAllOverlaps(Vec3 pos, float radius)
	{
		return FindAllOverlaps(FindCreateChunkOverlaps(pos, radius), pos, radius);
	}

	std::pair<vector<Entity*>, vector<Entity*>> FindPairOverlaps(vector<int> chunkOverlaps, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr) // Returns {successful, unsuccessful}
	{
		vector<Entity*> successful{}, unsuccessful{};
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter] && (*this)[*iter]->Overlaps(pos, radius) && (*this)[*iter]->active &&
					((func(from, (*this)[*iter].get()) && find(successful.begin(), successful.end(), (*this)[*iter].get()) == successful.end()) ||
						(!func(from, (*this)[*iter].get()) && find(unsuccessful.begin(), unsuccessful.end(), (*this)[*iter].get()) == unsuccessful.end())))
				{
					if ((*this)[*iter]->corporeal)
						successful.push_back((*this)[*iter].get());
					else
						unsuccessful.push_back((*this)[*iter].get());
				}
		return { successful, unsuccessful };
	}

	inline std::pair<vector<Entity*>, vector<Entity*>> FindPairOverlaps(Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		return FindPairOverlaps(FindCreateChunkOverlaps(pos, radius), pos, radius, func, from);
	}

	inline bool OverlapsAny(Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		return OverlapsTile(pos, radius) || DoesOverlap(pos, radius, func, from);
	}

	byte TileAtPos(Vec3 pos)
	{
		int index = ChunkAtPos(ToIV3(pos * (1.f / CHUNK_WIDTH)) * CHUNK_WIDTH);
		if (index == -1) return UnEnum(TILE::AIR);
		return chunks[index].TileAtPos(ToIV3(pos));
	}

	std::pair<Entity*, float> ExtremestOverlap(vector<int> chunkOverlaps, Vec3 pos, float radius, EntityMaskFun func, EntityExtremetyFun extr, Entity* from = nullptr)
	{
		std::pair<Entity*, float> result = { nullptr, 99999 };
		float tempExtr;
		Entity* tempEnt;
		for (int chunk : chunkOverlaps)
			for (int i : chunks[chunk])
			{
				tempEnt = (*this)[i].get();
				if (tempEnt != nullptr && tempEnt->Overlaps(pos, radius) && tempEnt->active && func(from, tempEnt) &&
					(tempExtr = extr(from, tempEnt)) < result.second) result = { tempEnt, tempExtr };
			}
		return result;
	}

	inline std::pair<Entity*, float> ExtremestOverlap(Vec3 pos, float radius, EntityMaskFun func, EntityExtremetyFun extr, Entity* from = nullptr)
	{
		return ExtremestOverlap(FindCreateChunkOverlaps(pos, radius), pos, radius, func, extr, from);
	}

	RaycastHit RaySphere(Vec3 ro, Vec3 rd, glm::vec4 sph, int index)
	{
		Vec3 oc = ro - Vec3(sph);
		float b = 2 * glm::dot(oc, rd);
		float c = glm::length2(oc) - sph.w * sph.w;
		float h = b * b - 4 * c;
		if (h < 0.0) return RaycastHit();
		RaycastHit hit;
		if (h == 0) hit.dist = -0.5f * b;
		else
		{
			float q = (b > 0) ?
				-0.5f * (b + sqrt(h)) :
				-0.5f * (b - sqrt(h));
			hit.dist = min(q, c / q);
		}
		hit.pos = ro + rd * hit.dist;
		hit.norm = glm::normalize(hit.pos - Vec3(sph));
		hit.chunkOrEntity = true;
		hit.index = index;
		return hit;
		// Old:
		/*Vec3 oc = ro - Vec3(sph);
		float b = glm::dot(oc, rd);
		float c = glm::dot(oc, oc) - sph.w * sph.w;
		float h = b * b - c;
		if (h < 0.0) return RaycastHit();
		h = sqrt(h);
		RaycastHit hit;
		hit.dist = -b - h;
		hit.pos = ro + rd * hit.dist;
		hit.norm = glm::normalize(hit.pos - Vec3(sph));
		hit.chunkOrEntity = true;
		hit.index = index;
		return hit;*/
	}

	RaycastHit RaycastEnt(Vec3 ro, Vec3 rd, float maxDist, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr) // Raycast can only hit entities
	{
		rd.x = rd.x == 0 ? 0.000001f : rd.x;
		rd.y = rd.y == 0 ? 0.000001f : rd.y;
		rd.z = rd.z == 0 ? 0.000001f : rd.z;
		Vec3 pos = Chunk::ToSpace(ro) * CHUNK_WIDTH;

		Vec3 step = glm::sign(rd);
		Vec3 tDelta = step / rd;
		RaycastHit hit = RaycastHit();
		
		float tMaxX, tMaxY, tMaxZ;

		Vec3 fr = glm::fract(ro / float(CHUNK_WIDTH));

		tMaxX = tDelta.x * ((rd.x > 0) ? (1 - fr.x) : fr.x);
		tMaxY = tDelta.y * ((rd.y > 0) ? (1 - fr.y) : fr.y);
		tMaxZ = tDelta.z * ((rd.z > 0) ? (1 - fr.z) : fr.z);

		float currentDist = 0;
		Vec3 step2 = step * float(CHUNK_WIDTH);

		const int maxTrace = 100;
		for (int i = 0; currentDist < maxDist && i < maxTrace; i++)
		{
			int chunk = ChunkAtPos(ToIV3(pos));
			if (chunk == -1) break;
			for (int index : chunks[chunk])
			{
				if (!func(from, (*this)[index].get())) continue;
				RaycastHit newHit = RaySphere(ro, rd, glm::vec4((*this)[index]->pos, (*this)[index]->radius), index);
				if (newHit.dist < hit.dist)
					hit = newHit;
			}

			if (tMaxX < tMaxY) {
				if (tMaxZ < tMaxX) {
					tMaxZ += tDelta.z;
					pos.z += step2.z;
					currentDist = tMaxZ * CHUNK_WIDTH;
				}
				else {
					tMaxX += tDelta.x;
					pos.x += step2.x;
					currentDist = tMaxX * CHUNK_WIDTH;
				}
			}
			else {
				if (tMaxZ < tMaxY) {
					tMaxZ += tDelta.z;
					pos.z += step2.z;
					currentDist = tMaxZ * CHUNK_WIDTH;
				}
				else {
					tMaxY += tDelta.y;
					pos.y += step2.y;
					currentDist = tMaxY * CHUNK_WIDTH;
				}
			}
		}
		return hit.dist < maxDist ? hit : RaycastHit();
	}

	RaycastHit StepCastChunk(Vec3 ro, Vec3 step, float maxDist)
	{
		iVec3 iPos = ToIV3(ro);
		iVec3 chunkPos = iPos / CHUNK_WIDTH;
		int chunkIndex = ChunkAtPos(chunkPos);
		iVec3 pos = iPos;
		float dist = 0;
		while (dist < maxDist)
		{

		}
	}
#pragma endregion
#pragma region Damage stuff
	int TryDealDamage(int damage, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr) // Can't hit tiles!!!
	{
		vector<Entity*> hitEntities = FindOverlaps(pos, radius, func, from);
		for (Entity* entity : hitEntities)
			return 2 + entity->ApplyHit(damage, from);
		return 0;
	}

	int TryDealDamageAll(int damage, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr) // Can't hit tiles!!!
	{
		int result = 0;
		vector<Entity*> hitEntities = FindOverlaps(pos, radius, func, from);
		for (Entity* entity : hitEntities)
			result = 2 + entity->ApplyHit(damage, from);
		return result;
	}

	int TryDealDamageAllUp(int damage, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr) // Can't hit tiles!!!
	{
		int result = 0;
		vector<Entity*> hitEntities = FindOverlaps(pos, radius, func, from);
		for (Entity* entity : hitEntities)
			if (entity->pos.z + entity->radius > pos.z)
			result = 2 + entity->ApplyHit(damage, from);
		return result;
	}

	int TryDealDamageTiles(int damage, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		vector<int> chunkOverlaps = game->entities->FindCreateChunkOverlaps(pos, radius);
		for (int i : chunkOverlaps)
		{
			iVec3 minPos = glm::max(vZeroI, ToIV3(pos - radius) - game->entities->chunks[i].pos);
			iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - game->entities->chunks[i].pos);
			for (int x = minPos.x; x <= maxPos.x; x++)
				for (int y = minPos.y; y <= maxPos.y; y++)
					for (int z = minPos.z; z <= maxPos.z; z++)
						if (game->entities->chunks[i].TileAtCPos(iVec3(x, y, z)) != UnEnum(TILE::AIR) &&
							BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, z) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
						{
							Chunk::DamageTile(damage, i, x, y, z);
							return 1;
						}
		}

		vector<Entity*> hitEntities = FindOverlaps(chunkOverlaps, pos, radius, func, from);
		for (Entity* entity : hitEntities)
			return 2 + entity->ApplyHit(damage, from);
		return 0;
	}

	int TryDealDamageAllTiles(int damage, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		int result = 0;
		vector<int> chunkOverlaps = game->entities->FindCreateChunkOverlaps(pos, radius);
		for (int i : chunkOverlaps)
		{
			iVec3 minPos = glm::max(vZeroI, ToIV3(pos - radius) - game->entities->chunks[i].pos);
			iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - game->entities->chunks[i].pos);
			for (int x = minPos.x; x <= maxPos.x; x++)
				for (int y = minPos.y; y <= maxPos.y; y++)
					for (int z = minPos.z; z <= maxPos.z; z++)
						if (game->entities->chunks[i].TileAtCPos(iVec3(x, y, z)) != UnEnum(TILE::AIR) &&
							BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, z) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
						{
							Chunk::DamageTile(damage, i, x, y, z);
							result = 1;
						}
		}

		vector<Entity*> hitEntities = FindOverlaps(chunkOverlaps, pos, radius, func, from);
		for (Entity* entity : hitEntities)
			result = 2 + entity->ApplyHit(damage, from);
		return result;
	}

	int TryDealDamageAllUpTiles(int damage, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		int result = 0;
		vector<int> chunkOverlaps = game->entities->FindCreateChunkOverlaps(pos, radius);
		for (int i : chunkOverlaps)
		{
			iVec3 minPos = glm::max(vZeroI, ToIV3(pos - Vec3(radius, radius, 0)) - game->entities->chunks[i].pos);
			iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - game->entities->chunks[i].pos);
			for (int x = minPos.x; x <= maxPos.x; x++)
				for (int y = minPos.y; y <= maxPos.y; y++)
					for (int z = minPos.z; z <= maxPos.z; z++)
						if (game->entities->chunks[i].TileAtCPos(iVec3(x, y, z)) != UnEnum(TILE::AIR) &&
							BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, z) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
						{
							Chunk::DamageTile(damage, i, x, y, z);
							result = 1;
						}
		}

		vector<Entity*> hitEntities = FindOverlaps(chunkOverlaps, pos, radius, func, from);
		for (Entity* entity : hitEntities)
			if (entity->pos.z + entity->radius > pos.z)
				result = 2 + entity->ApplyHit(damage, from);
		return result;
	}
#pragma endregion
// Add more overlap functions.
#pragma endregion
#pragma region Update stuff
	void SortEntities()
	{
		int ncCount = static_cast<int>(size()), collectibleCount = ncCount;
		for (unique_ptr<Entity>& entity : *this)
		{
			if (!entity)
			{
				ncCount--;
				collectibleCount--;
				continue;
			}
			ncCount -= int(entity->isCollectible);
			collectibleCount -= int(!entity->isCollectible);
		}
		vector<EntityIndex> unsortedToSorted = vector<EntityIndex>(ncCount);
		for (int i = 0, j = 0; i < size(); i++)
			if ((*this)[i] && !(*this)[i]->isCollectible)
			{
				unsortedToSorted[j] = EntityIndex(i, (*this)[i]->sortLayer);
				j++;
			}
		std::sort(unsortedToSorted.begin(), unsortedToSorted.end());

		sortedNCEntities = vector<Entity*>(ncCount);
		for (int i = 0, j = 0; i < size(); i++)
			if ((*this)[i] && !(*this)[i]->isCollectible)
			{
				sortedNCEntities[j] = (*this)[unsortedToSorted[j].index].get();
				j++;
			}

		collectibles = vector<Entity*>(collectibleCount);
		int i = 0;
		for (unique_ptr<Entity>& entity : *this)
			if (entity && entity->isCollectible)
				collectibles[i++] = entity.get();
		addedEntity = false;
	}

	void Update()
	{
		createdChunkThisFrame = false;

		for (int i = 0; i < toAddEntities.size(); i++)
			push_back(std::move(toAddEntities[i]));
		toAddEntities.clear();

		if (addedEntity)
			SortEntities();

		for (int i = 0; i < collectibles.size(); i++)
			if (collectibles[i]->active)
			{
				collectibles[i]->Update();
			}

		if (addedEntity)
			SortEntities();

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->active)
			{
				Entity* entity = sortedNCEntities[index];
				entity->Update();
			}

		for (int i = 0; i < toDelEntities.size(); i++)
			Remove(toDelEntities[i]);
		toDelEntities.clear();

		for (int i = 0; i < size(); i++)
			if ((*this)[i] && (*this)[i]->active)
				(*this)[i]->VUpdate();

		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->active && collectibles[index]->corporeal)
				collectibles[index]->UpdateChunkCollision();

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->active && sortedNCEntities[index]->corporeal)
				sortedNCEntities[index]->UpdateCollision();

		for (int i = 0; i < size(); i++)
			if ((*this)[i] && (*this)[i]->active)
				for (int j = 0; (*this)[i] && j < (*this)[i]->statuses.size(); j++)
				{
					StatusEffect temp = (*this)[i]->statuses[j];
					if (!temp.Update() || !(*this)[i])
					{
						if ((*this)[i])
							(*this)[i]->statuses[j] = temp;
						continue;
					}
					(*this)[i]->statuses.erase((*this)[i]->statuses.begin() + j);
					j--;

				}

		if (addedEntity)
			SortEntities();
	}

	void DUpdate()
	{
		std::pair<iVec3, iVec3> minMaxPos = Chunk::MinMaxPos(game->PlayerPos(), float(game->settings.chunkRenderDist * CHUNK_WIDTH));
		vector<int> chunkOverlaps = {};
		for (int i = 0, x = minMaxPos.first.x; x <= minMaxPos.second.x; x++)
			for (int y = minMaxPos.first.y; y <= minMaxPos.second.y; y++)
				for (int z = minMaxPos.first.z; z <= minMaxPos.second.z; z++)
				{
					Vec3 centerPos = Vec3(
						x * CHUNK_WIDTH + CHUNK_WIDTH * 0.5f,
						y * CHUNK_WIDTH + CHUNK_WIDTH * 0.5f,
						z * CHUNK_WIDTH + CHUNK_WIDTH * 0.5f);
					if (glm::distance2(centerPos, game->PlayerPos()) <=
						CHUNK_WIDTH * CHUNK_WIDTH * game->settings.chunkRenderDist * game->settings.chunkRenderDist &&
						game->camFrustum.BoxInFrustum(centerPos, Vec3(CHUNK_WIDTH * 0.5f)))
					{
						int chunk = ChunkAtPos(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, z * CHUNK_WIDTH));
						if (chunk != -1)
						{
							chunkOverlaps.push_back(chunk);
							continue;
						}
						else if (!createdChunkThisFrame)
						{
							chunks.push_back(Chunk(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, z * CHUNK_WIDTH)));
							chunks[chunks.size() - 1].Finalize();
						}
					}
					else
					{
						if (ChunkAtPos(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, z * CHUNK_WIDTH)) == -1 && !createdChunkThisFrame)
						{
							chunks.push_back(Chunk(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, z * CHUNK_WIDTH)));
							chunks[chunks.size() - 1].Finalize();
						}
					}
				}

		std::pair<vector<Entity*>, vector<Entity*>> toRenderPair = FindPairOverlaps(chunkOverlaps, game->PlayerPos(), static_cast<float>(game->settings.chunkRenderDist * CHUNK_WIDTH), MaskF::IsCorporealNotCollectible, nullptr);
		if (!game->inputs.keys[KeyCode::COMMA].held)
		{
			// Collectibles
			for (Entity* entity : toRenderPair.second)
			{
				entity->DUpdate();
			}
			// Normal entities
			for (Entity* entity : toRenderPair.first)
			{
				entity->DUpdate();
			}
			// Particles
			for (int i = 0; i < particles.size(); i++)
			{
				particles[i]->Update();
				if (particles[i]->ShouldEnd())
				{
					Particle* toDestroyParticle = particles[i].get();
					particles.erase(particles.begin() + i);
					i--;
				}
			}
			game->DrawAllCircles();
		}

		if (!game->inputs.keys[KeyCode::PERIOD].held)
		{
			// Chunks:
			glUseProgram(chunkShader);
			for (int i : chunkOverlaps)
				chunks[i].Draw();
		}
	}

	void UIUpdate()
	{
		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->dActive && collectibles[index]->uiActive)
			{
				collectibles[index]->UIUpdate();
			}

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->dActive && sortedNCEntities[index]->uiActive)
			{
				sortedNCEntities[index]->UIUpdate();
			}
	}
#pragma endregion
#pragma region Destruction stuff
	void Remove(Entity* entityToRemove)
	{
		// Remove from sortedNCEntities or from collectibles.
		if (!entityToRemove->isCollectible)
			sortedNCEntities.erase(std::find(sortedNCEntities.begin(), sortedNCEntities.end(), entityToRemove)); // It's a non-collectible.
		else
			collectibles.erase(std::find(collectibles.begin(), collectibles.end(), entityToRemove)); // It's a collectible.

		// Find where it's located in the main vector.
		vector<unique_ptr<Entity>>::iterator iter = std::find_if(begin(), end(), [entityToRemove](std::unique_ptr<Entity> const& i) { return i.get() == entityToRemove; });
		// The std::distance stuff finds its index.
		RemoveFromChunks(static_cast<int>(std::distance(begin(), iter)), entityToRemove->pos, entityToRemove->radius);
		// Replace its value with nullptr (that's what the .reset() does).
		iter->reset();
	}

	void RemoveLight(LightSource* lightSourceToRemove)
	{
		lightSources.erase(std::find_if(lightSources.begin(), lightSources.end(), [lightSourceToRemove](std::unique_ptr<LightSource> const& i) { return i.get() == lightSourceToRemove; }));
	}

	void RemoveDark(LightSource* lightSourceToRemove)
	{
		darkSources.erase(std::find_if(darkSources.begin(), darkSources.end(), [lightSourceToRemove](std::unique_ptr<LightSource> const& i) { return i.get() == lightSourceToRemove; }));
	}

	void Remove(Particle* particleToRemove)
	{
		particles.erase(std::find_if(particles.begin(), particles.end(), [particleToRemove](std::unique_ptr<Particle> const& i) { return i.get() == particleToRemove; }));
	}
#pragma endregion
#pragma region Helper functions
	// Returns sum of added velocities
	Vec3 VacuumBurst(Vec3 pos, float vacDist, float speed, float maxSpeed, bool vacBoth = false, bool vacCollectibles = true)
	{
		Vec3 result = vZero;
		for (unique_ptr<Entity>& entity : *this)
		{
			float distance;
			if (entity && (vacBoth || entity->isCollectible == vacCollectibles) && (entity->isCollectible || entity->corporeal) && entity->active &&
				(distance = glm::distance(pos, entity->pos)) > 0 && distance <= vacDist + entity->radius)
			{
				Vec3 oldVel = entity->vel;
				entity->vel = TryAdd2(entity->vel, Normalized(pos - entity->pos) * (speed / entity->mass), maxSpeed);
				result += entity->vel - oldVel;
			}
		}
		return result;
	}

	inline Vec3 Vacuum(Vec3 pos, float vacDist, float speed, float maxSpeed, bool vacBoth = false, bool vacCollectibles = true)
	{
		return VacuumBurst(pos, vacDist, speed * game->dTime, maxSpeed, vacBoth, vacCollectibles);
	}
#pragma endregion
};




#pragma region Post Entities functions

namespace HitResult
{
	enum
	{
		LIVED, DIED, MISSED = -1
	};
}
int Entity::ApplyHitHarmless(int damage, Entity* damageDealer) // 0 = lived, 1 = dead
{
	//if (damage > 0)
	//	game->entities->particles.push_back(make_unique<SpinText>(Vec3(pos) +
	//		Vec3(RandFloat(), RandFloat(), 0) * Vec3(radius * 2 - 1) - north, RandFloat() * 2 - 1, to_string(damage),
	//		RGBA(damageDealer->color.r, damageDealer->color.g, damageDealer->color.b, damageDealer->color.a / 2),
	//		static_cast<float>(COMMON_TEXT_SCALE), RandFloat() * 5.0f, COMMON_TEXT_SCALE * (RandFloat() * 0.25f + 0.25f)));

	health = min(health - damage, maxHealth);
	return health <= 0 ? HitResult::DIED : HitResult::LIVED;
}

void Entity::DestroySelf(Entity* damageDealer)
{
	if (uiActive)
		game->MenuedEntityDied(this);
	OnDeath(damageDealer);
	DetachObservers();
	game->entities->Remove(this);
}

void Entity::DelayedDestroySelf()
{
	DetachObservers();
	game->entities->DelayedDestroy(this);
}

void Entity::SetPos(Vec3 newPos)
{
	std::pair<iVec3, iVec3> minMaxOldPos = Chunk::MinMaxPos(pos, radius);
	std::pair<iVec3, iVec3> minMaxNewPos = Chunk::MinMaxPos(newPos, radius);
	if (minMaxOldPos.first != minMaxNewPos.first || minMaxOldPos.second != minMaxNewPos.second)
	{
		int position = static_cast<int>(distance(game->entities->begin(), std::find_if(game->entities->begin(), game->entities->end(), [this](std::unique_ptr<Entity> const& i) { return i.get() == this; })));
		vector<int> oldChunkOverlaps = game->entities->MainChunkOverlaps(minMaxOldPos.first, minMaxOldPos.second);
		for (int chunk : oldChunkOverlaps)
			game->entities->chunks[chunk].erase(find(game->entities->chunks[chunk].begin(), game->entities->chunks[chunk].end(), position));
		vector<int> newChunkOverlaps = game->entities->FindCreateChunkOverlapsMain(minMaxNewPos.first, minMaxNewPos.second);
		for (int chunk : newChunkOverlaps)
			game->entities->chunks[chunk].push_back(position);
	}
	pos = newPos;
}

void Entity::SetRadius(float newRadius)
{
	std::pair<iVec3, iVec3> minMaxOldRadius = Chunk::MinMaxPos(pos, radius);
	std::pair<iVec3, iVec3> minMaxNewRadius = Chunk::MinMaxPos(pos, newRadius);
	if (minMaxOldRadius.first != minMaxNewRadius.first || minMaxOldRadius.second != minMaxNewRadius.second)
	{
		int position = static_cast<int>(distance(game->entities->begin(), std::find_if(game->entities->begin(), game->entities->end(), [this](std::unique_ptr<Entity> const& i) { return i.get() == this; })));
		vector<int> oldChunkOverlaps = game->entities->MainChunkOverlaps(minMaxOldRadius.first, minMaxOldRadius.second);
		for (int chunk : oldChunkOverlaps)
			game->entities->chunks[chunk].erase(find(game->entities->chunks[chunk].begin(), game->entities->chunks[chunk].end(), position));
		vector<int> newChunkOverlaps = game->entities->FindCreateChunkOverlapsMain(minMaxNewRadius.first, minMaxNewRadius.second);
		for (int chunk : newChunkOverlaps)
			game->entities->chunks[chunk].push_back(position);
	}
	radius = newRadius;
}

void Entity::UpdateChunkCollision()
{
	vector<Vec3> hitPositions{};
	//bool overlappedAllPossible = true;
	vector<int> chunkOverlaps = game->entities->FindCreateChunkOverlaps(pos, radius);
	for (int i : chunkOverlaps)
	{
		iVec3 minPos = glm::max(vZeroI, ToIV3(pos - radius) - game->entities->chunks[i].pos);
		iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - game->entities->chunks[i].pos);
		for (int x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				for (int z = minPos.z; z <= maxPos.z; z++)
				{
					if (game->entities->chunks[i].TileAtCPos(iVec3(x, y, z)) != UnEnum(TILE::AIR))
					{
						if (ToIV3(pos) == iVec3(x, y, z) + game->entities->chunks[i].pos)
							return SetPos(pos + up);
						if (BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, z) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
							hitPositions.push_back(Vec3(iVec3(x, y, z) + game->entities->chunks[i].pos) + 0.5f);
					}
					//else
						//overlappedAllPossible = false;
				}
	}
	if (hitPositions.size() && !game->inputs.keys[KeyCode::PHASE].held)
	{
		//if (overlappedAllPossible)
		//	return SetPos(pos + up);

		Vec3 hitPosition = hitPositions[0];
		float distance = glm::length2(hitPosition - pos), newDistance;
		for (int i = 1; i < hitPositions.size(); i++)
			if ((newDistance = glm::length2(hitPositions[i] - pos)) < distance)
			{
				hitPosition = hitPositions[i];
				distance = newDistance;
			}

		Vec3 p = pos - hitPosition;
		Vec3 d = glm::abs(p) - 0.5f;
		float  m = glm::min(0.0f, glm::max(d.x, max(d.y, d.z)));
		Vec3 nearestPoint = p - Vec3(d.x >= m ? d.x : 0.0f, // Relative to box
			d.y >= m ? d.y : 0.0f,
			d.z >= m ? d.z : 0.0f) * sign(p);

		Vec3 normal = p - nearestPoint;
		float dist = glm::length(normal);
		if (dist == 0)
			return SetPos(pos + up);
		normal /= dist;
		Vec3 offset = (dist - radius) * normal;
		SetPos(pos - offset);
		vel = Lerp(vel * (vOne - glm::abs(normal)), glm::reflect(vel, normal), bounciness);
	}
}

void Entity::UpdateCollision()
{
	UpdateChunkCollision();

	vector<Entity*> entities = game->entities->FindOverlaps(pos, radius, MaskF::IsCorporealNotCollectible, this);
	for (Entity* entity : entities)
		OverlapRes::CircleOR(this, entity);
}

bool TileData::Damage(int damage)
{
	damageDealt += damage;
	if (damageDealt >= tileHealths[game->entities->chunks[chunkIndex].tiles[x][y][z]])
	{
		game->entities->chunks[chunkIndex].tiles[x][y][z] = UnEnum(TILE::AIR);
		game->entities->chunks[chunkIndex].RegenerateMesh();
		return true;
	}
	lastDamage = tTime;
	return false;
}

#pragma endregion


#pragma region Post entities definition entities

#define EXPLOSION_PARTICLE_COUNT 25
#define EXPLOSION_PARTICLE_SPEED 16.0f
#define EXPLOSION_PARTICLE_DURATION 0.5f
EntityData explodeNextFrameData = EntityData(UPDATE::EXPLODENEXTFRAME);
class ExplodeNextFrame : public Entity
{
public:
	int damage;
	float explosionRadius;
	float startTime;

	ExplodeNextFrame(EntityData* data, int damage = 1, float explosionRadius = 0.5f, RGBA color = RGBA(), Vec3 pos = vZero, string name = "NULL NAME", Entity* creator = nullptr) :
		Entity(data, pos, 0.5f, color, 1, 0, 1, 1, string("Explosion from ") + name, creator == nullptr ? Allegiance(0) : creator->allegiance),
		damage(damage), explosionRadius(explosionRadius), startTime(tTime)
	{
		this->creator = creator;
		corporeal = false;
	}
};

EntityData upExplodeNextFrameData = EntityData(UPDATE::UPEXPLODENEXTFRAME);
class UpExplodeNextFrame : public Entity
{
public:
	int damage;
	float explosionRadius;
	float startTime;

	UpExplodeNextFrame(EntityData* data, int damage = 1, float explosionRadius = 0.5f, RGBA color = RGBA(), Vec3 pos = vZero, string name = "NULL NAME", Entity* creator = nullptr) :
		Entity(data, pos, 0.5f, color, 1, 0, 1, 1, string("Explosion from ") + name, creator == nullptr ? Allegiance(0) : creator->allegiance),
		damage(damage), explosionRadius(explosionRadius), startTime(tTime)
	{
		this->creator = creator;
		corporeal = false;
	}
};

EntityData fadeOutPuddleData = EntityData(UPDATE::FADEOUTPUDDLE, VUPDATE::ENTITY, DUPDATE::FADEOUTPUDDLE);
class FadeOutPuddle : public Entity
{
public:
	int damage;
	float startTime, totalFadeTime, timePer, lastTime;

	FadeOutPuddle(EntityData* data, float totalFadeTime = 1.0f, int damage = 1, float timePer = 1.0f, Vec3 pos = vZero,
		float radius = 0.5f, RGBA color = RGBA()) :
		Entity(data, pos, radius, color, 1, 0, 1, 1, "Puddle"),
		totalFadeTime(totalFadeTime), damage(damage), startTime(tTime), timePer(timePer), lastTime(tTime)
	{
		corporeal = false;
	}

	FadeOutPuddle(FadeOutPuddle* baseClass, Vec3 pos, Entity* creator) :
		FadeOutPuddle(*baseClass) {
		this->pos = pos;
		startTime = tTime;
		if (creator != nullptr)
		{
			this->creator = creator;
			allegiance = creator->allegiance;
		}
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr) override
	{
		return make_unique<FadeOutPuddle>(this, pos, creator);
	}
};

EntityData fadeOutGlowData = EntityData(UPDATE::FADEOUT, VUPDATE::ENTITY, DUPDATE::FADEOUTGLOW, UIUPDATE::ENTITY, ONDEATH::FADEOUTGLOW);
class FadeOutGlow : public FadeOut
{
public:
	float startRange;
	LightSource* lightSource = nullptr;

	FadeOutGlow(EntityData* data, float range, float totalFadeTime = 1.0f, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA()) :
		FadeOut(data, totalFadeTime, pos, radius, color), startRange(range)
	{
		if (game && game->entities)
		{
			game->entities->lightSources.push_back(make_unique<LightSource>(pos, JRGB(color.r, color.g, color.b), range));
			lightSource = game->entities->lightSources[game->entities->lightSources.size() - 1].get();
		}
	}
};

EntityData vacuumForData = EntityData(UPDATE::VACUUMFOR);
class VacuumFor : public FadeOut
{
public:
	bool vacBoth, vacCollectibles;
	float vacDist, vacSpeed, maxVacSpeed;

	VacuumFor(EntityData* data, Vec3 pos, bool vacBoth, bool vacCollectibles, float timeTill, float vacDist, float vacSpeed, float maxVacSpeed, RGBA color) :
		FadeOut(data, timeTill, pos, vacDist, color),
		vacBoth(vacBoth), vacCollectibles(vacCollectibles), vacDist(vacDist), vacSpeed(vacSpeed), maxVacSpeed(maxVacSpeed)
	{
		corporeal = false;
	}

	VacuumFor(VacuumFor* baseClass, Vec3 pos) :
		VacuumFor(*baseClass)
	{
		this->pos = pos;
		startTime = tTime;
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr) override
	{
		return make_unique<VacuumFor>(this, pos);
	}
};

namespace Updates
{
	void CollectibleU(Entity* entity)
	{
		Collectible* collectible = static_cast<Collectible*>(entity);
		vector<Entity*> overlaps = game->entities->FindOverlaps(collectible->pos, collectible->radius, MaskF::IsCollectible, collectible);
		for (Entity* rawHit : overlaps)
		{
			Collectible* hit = static_cast<Collectible*>(rawHit);
			// Skip if they are different types:
			if (hit->baseItem.type != collectible->baseItem.type) continue;
			// Weightedly average the positions:
			collectible->SetPos((collectible->pos * float(collectible->baseItem.count) +
				hit->pos * float(hit->baseItem.count)) * (1.f / (collectible->baseItem.count + hit->baseItem.count)));
			// Prepare all observers in hit for transfer to the other collectible:
			for (Entity* observer : hit->observers)
				observer->MoveAttachment(hit, collectible);
			// Prep for concatenation:
			collectible->observers.reserve(collectible->observers.size() + hit->observers.size());
			// Concatenate the hit collectible's observers list onto the original collectible's observers list:
			collectible->observers.insert(collectible->observers.end(), hit->observers.begin(), hit->observers.end());
			// Increase the count of the original collectible:
			collectible->baseItem.count += hit->baseItem.count;
			// Destroy the hit collectible:
			hit->DestroySelf(collectible);
			// Modify the radius of the original collectible:
			collectible->SetRadius(collectible->FindRadius());
		}
	}

	void ExplodeNextFrameU(Entity* entity)
	{
		ExplodeNextFrame* explosion = static_cast<ExplodeNextFrame*>(entity);
		if (tTime != explosion->startTime)
		{
			game->entities->TryDealDamageAll(explosion->damage, explosion->pos, explosion->explosionRadius, MaskF::IsNonAlly, explosion);
			for (int i = 0; i < EXPLOSION_PARTICLE_COUNT; i++)
			{
				float rotation = RandFloat() * PI_F * 2;
				game->entities->particles.push_back(make_unique<VelocityCircle>(0.25f, explosion->pos, Vec3(sinf(rotation), cosf(rotation), 0) * EXPLOSION_PARTICLE_SPEED,
					explosion->color, EXPLOSION_PARTICLE_DURATION));
			}
			explosion->DestroySelf(explosion);
		}
	}

	void UpExplodeNextFrameU(Entity* entity)
	{
		UpExplodeNextFrame* explosion = static_cast<UpExplodeNextFrame*>(entity);
		if (tTime != explosion->startTime)
		{
			game->entities->TryDealDamageAllUp(explosion->damage, explosion->pos, explosion->explosionRadius, MaskF::IsNonAlly, explosion);
			for (int i = 0; i < EXPLOSION_PARTICLE_COUNT; i++)
			{
				float rotation = RandFloat() * PI_F * 2;
				game->entities->particles.push_back(make_unique<VelocityCircle>(0.25f, explosion->pos, Vec3(sinf(rotation), cosf(rotation), 0) * EXPLOSION_PARTICLE_SPEED,
					explosion->color, EXPLOSION_PARTICLE_DURATION));
			}
			explosion->DestroySelf(explosion);
		}
	}

	void FadeOutPuddleU(Entity* entity)
	{
		FadeOutPuddle* puddle = static_cast<FadeOutPuddle*>(entity);
		if (tTime - puddle->lastTime > puddle->timePer)
		{
			puddle->lastTime = tTime;
			game->entities->TryDealDamageAll(puddle->damage, puddle->pos, puddle->radius, MaskF::IsNonAlly, puddle);
		}
		if (tTime - puddle->startTime > puddle->totalFadeTime)
			puddle->DestroySelf(puddle);
	}

	void VacuumeForU(Entity* entity)
	{
		VacuumFor* vac = static_cast<VacuumFor*>(entity);

		if (tTime - vac->startTime > vac->totalFadeTime)
		{
			vac->DestroySelf(nullptr);
			return;
		}

		game->entities->Vacuum(vac->pos, vac->vacDist, vac->vacSpeed, vac->maxVacSpeed, vac->vacBoth, vac->vacCollectibles);
	}
}

namespace DUpdates
{
	void FadeOutPuddleDU(Entity* entity)
	{
		FadeOutPuddle* puddle = static_cast<FadeOutPuddle*>(entity);
		uint a = puddle->color.a;
		puddle->color.a *= static_cast<uint8_t>((tTime - puddle->startTime) * 255 / puddle->totalFadeTime);
		puddle->DUpdate(DUPDATE::ENTITY);
		puddle->color.a = a;
	}

	void FadeOutGlowDU(Entity* entity)
	{
		FadeOutGlow* glow = static_cast<FadeOutGlow*>(entity);
		glow->lightSource->range = glow->startRange * glow->Opacity();
		glow->DUpdate(DUPDATE::FADEOUT);
	}
}

namespace OnDeaths
{
	void FadeOutGlowOD(Entity* entity, Entity* damageDealer)
	{
		game->entities->RemoveLight(((FadeOutGlow*)entity)->lightSource);
	}
}

#pragma endregion

#pragma region Post entities definition items

class PlacedOnLanding : public Item
{
public:
	bool sayCreator;
	Entity* entityToPlace;
	string creatorName;

	PlacedOnLanding(ITEMTYPE type, Entity* entityToPlace, string typeName, VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0, int damage = 0, float range = 15.0f, bool sayCreator = false,
		float useTime = 0.25f, float speed = 12, float radius = 0.4f, bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		Item(type, entityToPlace->name, typeName, vUpdate, intType, entityToPlace->color, damage, range, useTime, speed, radius,
			corporeal, shouldCollide, collideTerrain, mass, health), entityToPlace(entityToPlace), sayCreator(sayCreator)
	{
		itemOD = ITEMOD::PLACEDONLANDING;
	}

	PlacedOnLanding(ITEMTYPE type, Entity* entityToPlace, string name, string typeName, VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0, RGBA color = RGBA(),
		int damage = 1, float range = 15.0f, bool sayCreator = false, float useTime = 0.25f, float speed = 12, float radius = 0.4f,
		bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		Item(type, name, typeName, vUpdate, intType, color, damage, range, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health),
		entityToPlace(entityToPlace), sayCreator(sayCreator)
	{
		itemOD = ITEMOD::PLACEDONLANDING;
	}
};

class CorruptOnKill : public PlacedOnLanding
{
public:
	CorruptOnKill(ITEMTYPE type, Entity* entityToPlace, string typeName, VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0, int damage = 0, float range = 15.0f, bool sayCreator = false,
		float useTime = 0.25f, float speed = 12, float radius = 0.4f, bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		PlacedOnLanding(type, entityToPlace, typeName, vUpdate, intType, damage, range, sayCreator,
			useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health)
	{
		itemOD = ITEMOD::CORRUPTONKILL;
	}

	CorruptOnKill(ITEMTYPE type, Entity* entityToPlace, string name, string typeName, VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0, RGBA color = RGBA(),
		int damage = 1, float range = 15.0f, bool sayCreator = false, float useTime = 0.25f, float speed = 12, float radius = 0.4f,
		bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		PlacedOnLanding(type, entityToPlace, name, typeName, vUpdate, intType, color, damage, range, sayCreator, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health)
	{
		itemOD = ITEMOD::CORRUPTONKILL;
	}
};

class PlacedOnLandingBoom : public PlacedOnLanding
{
public:
	float explosionRadius;
	int explosionDamage;

	PlacedOnLandingBoom(ITEMTYPE type, float explosionRadius, int explosionDamage, Entity* entityToPlace, string typeName, VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0, int damage = 0, float range = 15.0f, bool sayCreator = false,
		float useTime = 0.25f, float speed = 12, float radius = 0.4f, bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		PlacedOnLanding(type, entityToPlace, typeName, vUpdate, intType, damage, range, sayCreator,
			useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health), explosionRadius(explosionRadius), explosionDamage(explosionDamage)
	{
		itemOD = ITEMOD::PLACEDONLANDINGBOOM;
	}

	PlacedOnLandingBoom(ITEMTYPE type, float explosionRadius, int explosionDamage, Entity* entityToPlace, string name, string typeName, VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0, RGBA color = RGBA(),
		int damage = 1, float range = 15.0f, bool sayCreator = false, float useTime = 0.25f, float speed = 12, float radius = 0.4f,
		bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		PlacedOnLanding(type, entityToPlace, name, typeName, vUpdate, intType, color, damage, range, sayCreator, useTime, speed, radius,
			corporeal, shouldCollide, collideTerrain, mass, health), explosionRadius(explosionRadius), explosionDamage(explosionDamage)
	{
		itemOD = ITEMOD::PLACEDONLANDINGBOOM;
	}
};

inline void CreateExplosion(Vec3 pos, float explosionRadius, RGBA color, string name, int damage, int explosionDamage, Entity* creator)
{
	game->entities->push_back(make_unique<ExplodeNextFrame>(&explodeNextFrameData, explosionDamage, explosionRadius, color, pos, name, creator));
	game->entities->push_back(make_unique<FadeOutGlow>(&fadeOutGlowData, explosionRadius * 2.0f, 2.f, pos, explosionRadius, color));
}

inline void CreateUpExplosion(Vec3 pos, float explosionRadius, RGBA color, string name, int damage, int explosionDamage, Entity* creator)
{
	game->entities->push_back(make_unique<UpExplodeNextFrame>(&upExplodeNextFrameData, explosionDamage, explosionRadius, color, pos, name, creator));
	game->entities->push_back(make_unique<FadeOutGlow>(&fadeOutGlowData, explosionRadius * 2.0f, 2.f, pos, explosionRadius, color));
}

class ExplodeOnLanding : public Item
{
public:
	int explosionDamage;
	float explosionRadius;

	ExplodeOnLanding(ITEMTYPE type, float explosionRadius = 0.5f, int explosionDamage = 1, string name = "NULL", string typeName = "NULL TYPE", VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0,
		RGBA color = RGBA(), int damage = 1, float range = 15.0f, float useTime = 0.25f, float speed = 12, float radius = 0.4f,
		bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		Item(type, name, typeName, vUpdate, intType, color, damage, range, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health),
		explosionRadius(explosionRadius), explosionDamage(explosionDamage)
	{
		itemOD = ITEMOD::EXPLODEONLANDING;
	}
};

class UpExplodeOnLanding : public ExplodeOnLanding
{
public:
	UpExplodeOnLanding(ITEMTYPE type, float explosionRadius = 0.5f, int explosionDamage = 1, string name = "NULL", string typeName = "NULL TYPE", VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0,
		RGBA color = RGBA(), int damage = 1, float range = 15.0f, float useTime = 0.25f, float speed = 12, float radius = 0.4f,
		bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		ExplodeOnLanding(type, explosionRadius, explosionDamage, name, typeName, vUpdate, intType, color, damage, range, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health)
	{
		itemOD = ITEMOD::UPEXPLODEONLANDING;
	}
};

class ImproveSoilOnLanding : public Item
{
public:
	int improveRadius; // Used like a square's radius.
	ImproveSoilOnLanding(ITEMTYPE type, int improveRadius, string name = "NULL", string typeName = "NULL TYPE", VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0,
		RGBA color = RGBA(), int damage = 1, float range = 15.0f, float useTime = 0.25f, float speed = 12, float radius = 0.4f,
		bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		Item(type, name, typeName, vUpdate, intType, color, damage, range, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health),
		improveRadius(improveRadius)
	{
		itemOD = ITEMOD::IMPROVESOILONLANDING;
	}
};

class SetTileOnLanding : public Item
{
public:
	int zOffset;
	TILE tile;

	SetTileOnLanding(ITEMTYPE type, TILE tile, int zOffset, string name = "NULL", string typeName = "NULL TYPE", VUPDATE vUpdate = VUPDATE::ENTITY, int intType = 0,
		RGBA color = RGBA(), int damage = 1, float range = 15.0f, float useTime = 0.25f, float speed = 12, float radius = 0.4f,
		bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		Item(type, name, typeName, vUpdate, intType, color, damage, range, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health),
		tile(tile), zOffset(zOffset)
	{
		itemOD = ITEMOD::SETTILEONLANDING;
	}
};

namespace ItemODs
{
	void ItemOD(ItemInstance& item, Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		unique_ptr<Collectible> collectible = make_unique<Collectible>(item.Clone(), pos);
		game->entities->push_back(std::move(collectible));
	}

	void PlacedOnLandingOD(ItemInstance& item, Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		PlacedOnLanding* pOL = static_cast<PlacedOnLanding*>(item.Type());

		unique_ptr<Entity> placedEntity = pOL->entityToPlace->Clone(pos, dir, creator);
		if (pOL->sayCreator)
			placedEntity->name += " from " + creatorName;
		game->entities->push_back(std::move(placedEntity));
	}

	void CorruptOnKillOD(ItemInstance& item, Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		item->OnDeath(callType == 2 ? ITEMOD::PLACEDONLANDING : ITEMOD::DEFAULT, item, pos, dir, vel, creator, creatorName, callReason, callType);
	}

	void PlacedOnLandingBoomOD(ItemInstance& item, Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		PlacedOnLandingBoom* explosion = static_cast<PlacedOnLandingBoom*>(item.Type());
		CreateExplosion(pos, explosion->explosionRadius, explosion->color, explosion->name + string(" shot by " + creatorName),
			explosion->damage, explosion->explosionDamage, creator);
		explosion->OnDeath(ITEMOD::PLACEDONLANDING, item, pos, dir, vel, creator, creatorName, callReason, callType);
	}

	void ExplodeOnLandingOD(ItemInstance& item, Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		ExplodeOnLanding* explosion = static_cast<ExplodeOnLanding*>(item.Type());
		CreateExplosion(pos, explosion->explosionRadius, explosion->color, explosion->name + string(" shot by " + creatorName),
			explosion->damage, explosion->explosionDamage, creator);
	}

	void UpExplodeOnLandingOD(ItemInstance& item, Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		ExplodeOnLanding* explosion = static_cast<ExplodeOnLanding*>(item.Type());
		CreateUpExplosion(pos, explosion->explosionRadius, explosion->color, explosion->name + string(" shot by " + creatorName),
			explosion->damage, explosion->explosionDamage, creator);
	}

	void ImproveSoilOnLandingOD(ItemInstance& item, Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		ImproveSoilOnLanding* soilItem = static_cast<ImproveSoilOnLanding*>(item.Type());

		int offset = -soilItem->improveRadius / 2;
		for (int x = 0; x < soilItem->improveRadius; x++)
			for (int y = 0; y < soilItem->improveRadius; y++)
				for (int z = 0; z < soilItem->improveRadius; z++)
				{
					iVec3 currentPos = ToIV3(pos) + iVec3(x, y, z) + offset;
					Chunk* chunk = game->entities->ChunkAtFPos(currentPos);
					TILE tile = TILE(chunk->TileAtPos(currentPos));
					chunk->SetTileAtPos(currentPos, UnEnum(tile == TILE::ROCK ? TILE::SAND : tile == TILE::SAND ? TILE::BAD_SOIL : tile == TILE::BAD_SOIL ?
						TILE::MID_SOIL : tile == TILE::MID_SOIL ? TILE::MAX_SOIL : tile));
				}
	}

	void SetTileOnLandingOD(ItemInstance& item, Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		SetTileOnLanding* tileItem = static_cast<SetTileOnLanding*>(item.Type());

		pos.z += tileItem->zOffset;
		Chunk* chunk = game->entities->ChunkAtFPos(pos);
		chunk->SetTileAtPos(ToIV3(pos), UnEnum(tileItem->tile));
	}
}

namespace Hazards
{
	FadeOutPuddle leadPuddle = FadeOutPuddle(&fadeOutPuddleData, 3.0f, 10, 0.2f, vZero, 1.5f, RGBA(80, 43, 92));
	VacuumFor vacuumPuddle = VacuumFor(&vacuumForData, vZero, false, true, 5, 25, 16, 4, RGBA(255, 255, 255, 51));
}

namespace Resources
{
	SetTileOnLanding ruby = SetTileOnLanding(ITEMTYPE::RUBY, TILE::RUBY_SOIL, -1, "Ruby", "Tile", VUPDATE::ENTITY, 5, RGBA(168, 50, 100), 0, 15.f, 0.25f, 12.f, 0.5f, false, true, true);
	ExplodeOnLanding emerald = ExplodeOnLanding(ITEMTYPE::EMERALD, 7.5f, 60, "Emerald", "Ammo", VUPDATE::ENTITY, 1, RGBA(65, 224, 150), 0, 15, 0.25f, 12, 0.4f, false, true, true);
	ExplodeOnLanding topaz = ExplodeOnLanding(ITEMTYPE::TOPAZ, 3.5f, 30, "Topaz", "Ammo", VUPDATE::ENTITY, 1, RGBA(255, 200, 0), 0, 15.0f, 0.25f, 12, 1.5f, false, true, true);
	ExplodeOnLanding sapphire = ExplodeOnLanding(ITEMTYPE::SAPPHIRE, 1.5f, 10, "Sapphire", "Ammo", VUPDATE::ENTITY, 1, RGBA(78, 25, 212), 0, 15.0f, 0.125f, 12, 0.1f, false, true, true);
	PlacedOnLanding lead = PlacedOnLanding(ITEMTYPE::LEAD, &Hazards::leadPuddle, "Lead", "Ammo", VUPDATE::ENTITY, 1, RGBA(80, 43, 92), 0, 15.0f, true, 0.25f, 12, 0.4f, false, true, true);
	PlacedOnLanding vacuumium = PlacedOnLanding(ITEMTYPE::VACUUMIUM, &Hazards::vacuumPuddle, "Vacuumium", "Push Ammo", VUPDATE::ENTITY, 1, RGBA(255, 255, 255), 0, 15, false, 0.25f, 12, 0.1f, false, true, true);
	ImproveSoilOnLanding quartz = ImproveSoilOnLanding(ITEMTYPE::QUARTZ, 3, "Quartz", "Tile", VUPDATE::ENTITY, 5, RGBA(156, 134, 194), 0, 15, 0.125f, 12.f, 0.5f, false, true, true);
}

namespace Collectibles
{
	Collectible* ruby = new Collectible(Resources::ruby.Clone());
	Collectible* emerald = new Collectible(Resources::emerald.Clone());
	Collectible* topaz = new Collectible(Resources::topaz.Clone());
	Collectible* sapphire = new Collectible(Resources::sapphire.Clone());
	Collectible* lead = new Collectible(Resources::lead.Clone());
	Collectible* vacuumium = new Collectible(Resources::vacuumium.Clone());
	Collectible* quartz = new Collectible(Resources::quartz.Clone());
}

#pragma endregion