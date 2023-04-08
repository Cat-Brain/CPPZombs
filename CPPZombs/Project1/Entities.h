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

class LightSource;
class Entities : public vector<unique_ptr<Entity>>
{
protected:
	int index;

public:
	bool addedEntity;
	vector<Entity*> sortedNCEntities; // The NC stands for Non-Collectible.
	vector<Entity*> collectibles; // sortedNCEntities and collectibles are the most accurate, the others are less so.
	vector<unique_ptr<LightSource>> lightSources;
	vector<unique_ptr<LightSource>> darkSources;
	vector<unique_ptr<Particle>> particles;
	vector<unique_ptr<Entity>> toAddEntities;
	vector<Chunk> chunks;
	vector<Chunk*> renderedChunks;

	Entities() :
		vector(0), addedEntity(false), index(0)
	{
		Chunk::Init();
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

#pragma region Overlaps and collisionstuff
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

	vector<int> MainChunkOverlaps(iVec2 minPos, iVec2 maxPos) // In chunk coords, do NOT plug in normal space coords.
	{
		vector<int> result((maxPos.x - minPos.x + 1) * (maxPos.y - minPos.y + 1));
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				result[i++] = ChunkAtPos(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, 0));
		return result;
	}

	vector<int> FindCreateChunkOverlapsMain(iVec2 minPos, iVec2 maxPos) // A safer version of MainChunksOveraps.
	{
		vector<int> result((maxPos.x - minPos.x + 1) * (maxPos.y - minPos.y + 1));
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
			{
				int chunk = ChunkAtPos(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, 0));
				if (chunk != -1)
				{
					result[i++] = chunk;
					continue;
				}
				result[i++] = static_cast<int>(chunks.size());
				chunks.push_back(Chunk(iVec3(x * CHUNK_WIDTH, y * CHUNK_WIDTH, 0)));
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

	bool OverlapsTile(Vec3 pos, float radius)
	{
		vector<int> chunkOverlaps = FindCreateChunkOverlaps(pos, radius);
		for (int i : chunkOverlaps)
		{
			iVec3 minPos = glm::max(iVec3(0), ToIV3(pos - radius) - chunks[i].pos);
			iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - chunks[i].pos);
			for (int x = minPos.x; x <= maxPos.x; x++)
				for (int y = minPos.y; y <= maxPos.y; y++)
					if (chunks[i].TileAtCPos(Vec3(x, y, 1)) != UnEnum(TILE::AIR) && BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, 0) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
						return true;
		}
		return false;
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

	int TryDealDamage(int damage, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr) // Can hit tiles!!!
	{
		vector<int> chunkOverlaps = game->entities->FindCreateChunkOverlaps(pos, radius);
		for (int i : chunkOverlaps)
		{
			iVec3 minPos = glm::max(iVec3(0), ToIV3(pos - radius) - game->entities->chunks[i].pos);
			iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - game->entities->chunks[i].pos);
			for (int x = minPos.x; x <= maxPos.x; x++)
				for (int y = minPos.y; y <= maxPos.y; y++)
					if (game->entities->chunks[i].TileAtCPos(iVec3(x, y, 1)) != UnEnum(TILE::AIR) &&
						BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, 0) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
					{
						Chunk::DamageTile(damage, i, x, y);
						return 1;
					}
		}

		vector<Entity*> hitEntities = FindOverlaps(chunkOverlaps, pos, radius, func, from);
		for (Entity* entity : hitEntities)
			return 2 + entity->DealDamage(damage, from);
		return 0;
	}

	int TryDealDamageAll(int damage, Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr) // Can hit tiles!!!
	{
		int result = 0;
		vector<int> chunkOverlaps = game->entities->FindCreateChunkOverlaps(pos, radius);
		for (int i : chunkOverlaps)
		{
			iVec3 minPos = glm::max(iVec3(0), ToIV3(pos - radius) - game->entities->chunks[i].pos);
			iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - game->entities->chunks[i].pos);
			for (int x = minPos.x; x <= maxPos.x; x++)
				for (int y = minPos.y; y <= maxPos.y; y++)
					if (game->entities->chunks[i].TileAtCPos(iVec3(x, y, 1)) != UnEnum(TILE::AIR) &&
						BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, 0) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
					{
						Chunk::DamageTile(damage, i, x, y);
						result = 1;
					}
		}

		vector<Entity*> hitEntities = FindOverlaps(chunkOverlaps, pos, radius, func, from);
		for (Entity* entity : hitEntities)
			result = 2 + entity->DealDamage(damage, from);
		return result;
	}

	bool OverlapsAny(Vec3 pos, float radius, function<bool(Entity* from, Entity* to)> func, Entity* from = nullptr)
	{
		return OverlapsTile(pos, radius) || DoesOverlap(pos, radius, func, from);
	}

	// Add more overlap functions.
#pragma endregion

	byte TileAtPos(Vec3 pos)
	{
		int index = ChunkAtPos(ToIV3(pos * (1.f / CHUNK_WIDTH)) * CHUNK_WIDTH);
		return chunks[index].TileAtPos(ToIV3(pos));
	}

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
		for (int i = 0; i < size(); i++)
			if ((*this)[i] && (*this)[i]->active)
				(*this)[i]->VUpdate();
		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->active && sortedNCEntities[index]->corporeal)
			{
				Entity* entity = sortedNCEntities[index];
				entity->UpdateCollision();
			}

		if (addedEntity)
			SortEntities();
	}

	void DUpdate()
	{
		vector<int> chunkOverlaps = FindCreateChunkOverlaps(game->PlayerPos(), game->DistToCorner());
		for (int i : chunkOverlaps)
			chunks[i].Draw();

		std::pair<vector<Entity*>, vector<Entity*>> toRenderPair = FindPairOverlaps(chunkOverlaps, game->PlayerPos(), game->DistToCorner(), MaskF::IsCorporeal);
		// Collectibles
		for (Entity* entity : toRenderPair.second)
		{
			entity->EarlyDUpdate();
		}
		for (Entity* entity : toRenderPair.second)
		{
			entity->DUpdate();
		}
		// Normal entities
		for (Entity* entity : toRenderPair.first)
		{
			entity->EarlyDUpdate();
		}
		for (Entity* entity : toRenderPair.first)
		{
			entity->DUpdate();
		}

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

	void Remove(Entity* entityToRemove)
	{
		// Remove from sortedNCEntities or from collectibles.
		if (!entityToRemove->isCollectible)
			sortedNCEntities.erase(std::find(sortedNCEntities.begin(), sortedNCEntities.end(), entityToRemove)); // It's a non-collectible.
		else
			collectibles.erase(std::find(collectibles.begin(), collectibles.end(), entityToRemove)); // It's a collectible.

		// Find where it's located in the main vector and replace its value with nullptr (that's what the .reset() does).
		(*std::find_if(begin(), end(), [entityToRemove](std::unique_ptr<Entity> const& i) { return i.get() == entityToRemove; })).reset();
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

	void VacuumBurst(Vec3 pos, float vacDist, float speed, float maxSpeed, bool vacBoth = false, bool vacCollectibles = true)
	{
		for (unique_ptr<Entity>& entity : *this)
		{
			float distance;
			if (entity && (vacBoth || entity->isCollectible == vacCollectibles) && (entity->isCollectible || entity->corporeal) && entity->active &&
				(distance = glm::distance(pos, entity->pos)) > 0 && distance <= vacDist + entity->radius)
				entity->vel = TryAdd2(entity->vel, Normalized(pos - entity->pos) * (speed / entity->mass), maxSpeed);
		}
	}

	inline void Vacuum(Vec3 pos, float vacDist, float speed, float maxSpeed, bool vacBoth = false, bool vacCollectibles = true)
	{
		VacuumBurst(pos, vacDist, speed * game->dTime, maxSpeed, vacBoth, vacCollectibles);
	}
};

#pragma region Post Entities functions

int Entity::DealDamage(int damage, Entity* damageDealer)
{
	if (damage > 0)
		game->entities->particles.push_back(make_unique<SpinText>(Vec3(pos) + Vec3(RandFloat(), RandFloat(), 0) * Vec3(radius * 2 - 1) - up, static_cast<float>(damage), to_string(damage),
			RGBA(damageDealer->color.r, damageDealer->color.g, damageDealer->color.b, damageDealer->color.a / 2),
			static_cast<float>(COMMON_TEXT_SCALE), RandFloat() * 5.0f, COMMON_TEXT_SCALE * (RandFloat() * 0.25f + 0.25f)));

	health = min(health - damage, maxHealth);
	if (health <= 0)
	{
		DestroySelf(damageDealer);
		return 1;
	}
	return 0;
}

void Entity::DestroySelf(Entity* damageDealer)
{
	if (uiActive)
		game->MenuedEntityDied(this);
	OnDeath(damageDealer);
	for (Entity* entity : observers)
		entity->UnAttach(this);
	game->entities->Remove(this);
}

void Entity::SetPos(Vec3 newPos)
{
	newPos = Vec3(newPos.x, newPos.y, 0);
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
	std::pair<iVec2, iVec2> minMaxOldRadius = Chunk::MinMaxPos(pos, radius);
	std::pair<iVec2, iVec2> minMaxNewRadius = Chunk::MinMaxPos(pos, newRadius);
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

void Entity::UpdateCollision()
{
	vector<Vec3> hitPositions{};
	vector<int> chunkOverlaps = game->entities->FindCreateChunkOverlaps(pos, radius);
	for (int i : chunkOverlaps)
	{
		iVec3 minPos = glm::max(iVec3(0), ToIV3(pos - radius) - game->entities->chunks[i].pos);
		iVec3 maxPos = glm::min(iVec3(CHUNK_WIDTH - 1), ToIV3(pos + radius) - game->entities->chunks[i].pos);
		for (int x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				if (game->entities->chunks[i].TileAtCPos(iVec3(x, y, 1)) != UnEnum(TILE::AIR) &&
					BoxCircleOverlap(pos, radius, Vec3(iVec3(x, y, 0) + game->entities->chunks[i].pos) + 0.5f, Vec3(0.5f)))
					hitPositions.push_back(Vec3(iVec3(x, y, 0) + game->entities->chunks[i].pos) + 0.5f);
	}
	if (hitPositions.size() && !game->inputs.space.held)
	{
		Vec3 hitPosition = hitPositions[0];
		float distance = glm::length2(hitPosition - pos), newDistance;
		for (int i = 1; i < hitPositions.size(); i++)
			if ((newDistance = glm::length2(hitPositions[i] - pos)) < distance)
			{
				hitPosition = hitPositions[i];
				distance = newDistance;
			}
		Vec2 p = pos - hitPosition;
		Vec2 w = glm::abs(p) - 0.5f;
		Vec2 s = Vec2(p.x < 0.0 ? -1 : 1, p.y < 0.0 ? -1 : 1);

		float g = max(w.x, w.y);
		Vec2  q = glm::max(w, Vec2(0.0));
		float l = length(q);

		Vec2 normal = Vec2(s * ((g > 0.0) ? q / l : ((w.x > w.y) ? Vec2(1, 0) : Vec2(0, 1))));
		Vec2 offset = (((g > 0.0) ? l : g) - radius) * normal;
		SetPos(pos - Vec3(offset, 0));
		vel = glm::reflect(vel, Vec3(normal, 0));
	}


	vector<Entity*> entities = game->entities->FindOverlaps(pos, radius, MaskF::IsCorporeal);
	for (Entity* entity : entities)
		if (entity->pos != pos && entity->creator != this && creator != entity)
			OverlapRes::CircleOR(this, entity);
}

bool TileData::Damage(int damage)
{
	damageDealt += damage;
	if (damageDealt >= tileHealths[game->entities->chunks[chunkIndex].tiles[x][y][1]])
	{
		game->entities->chunks[chunkIndex].tiles[x][y][1] = UnEnum(TILE::AIR);
		return true;
	}
	lastDamage = tTime;
	return false;
}

#pragma endregion


// Post entities definition entities:

#define EXPLOSION_PARTICLE_COUNT 25
#define EXPLOSION_PARTICLE_SPEED 16.0f
#define EXPLOSION_PARTICLE_DURATION 0.5f
class ExplodeNextFrame : public Entity
{
public:
	int damage;
	float explosionRadius;
	float startTime;

	ExplodeNextFrame(int damage = 1, float explosionRadius = 0.5f, RGBA color = RGBA(), Vec3 pos = vZero, string name = "NULL NAME", Entity* creator = nullptr) :
		Entity(pos, 0.5f, color, 1, 1, 1, string("Explosion from ") + name), damage(damage), explosionRadius(explosionRadius), startTime(tTime)
	{
		update = UPDATE::EXPLODENEXTFRAME;
		this->creator = creator;
		corporeal = false;
	}
};

class FadeOutPuddle : public Entity
{
public:
	int damage;
	float startTime, totalFadeTime, timePer, lastTime;

	FadeOutPuddle(float totalFadeTime = 1.0f, int damage = 1, float timePer = 1.0f, Vec3 pos = vZero,
		float radius = 0.5f, RGBA color = RGBA()) :
		Entity(pos, radius, color, 1, 1, 1, "Puddle"),
		totalFadeTime(totalFadeTime), damage(damage), startTime(tTime), timePer(timePer), lastTime(tTime)
	{
		update = UPDATE::FADEOUTPUDDLE;
		dUpdate = DUPDATE::FADEOUTPUDDLE;
		corporeal = false;
	}

	FadeOutPuddle(FadeOutPuddle* baseClass, Vec3 pos) :
		FadeOutPuddle(*baseClass) {
		this->pos = pos;
		startTime = tTime;
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<FadeOutPuddle>(this, pos);
	}
};

class FadeOutGlow : public FadeOut
{
public:
	float startRange;
	LightSource* lightSource;

	FadeOutGlow(float range, float totalFadeTime = 1.0f, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA()) :
		FadeOut(totalFadeTime, pos, radius, color), startRange(range)
	{
		dUpdate = DUPDATE::FADEOUTGLOW;
		onDeath = ONDEATH::FADEOUTGLOW;
		game->entities->lightSources.push_back(make_unique<LightSource>(pos, JRGB(color.r, color.g, color.b), range));
		lightSource = game->entities->lightSources[game->entities->lightSources.size() - 1].get();
	}
};

class VacuumeFor : public FadeOut
{
public:
	float vacDist, vacSpeed, maxVacSpeed;

	VacuumeFor(Vec3 pos, float timeTill, float vacDist, float vacSpeed, float maxVacSpeed, RGBA color) :
		FadeOut(timeTill, pos, vacDist, color), vacDist(vacDist), vacSpeed(vacSpeed), maxVacSpeed(maxVacSpeed)
	{
		update = UPDATE::VACUUMEFOR;
		corporeal = false;
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<VacuumeFor>(pos, totalFadeTime, vacDist, vacSpeed, maxVacSpeed, color);
	}
};

namespace Updates
{
	void ExplodeNextFrameU(Entity* entity)
	{
		ExplodeNextFrame* explosion = static_cast<ExplodeNextFrame*>(entity);
		if (tTime != explosion->startTime)
		{
			game->entities->TryDealDamageAll(explosion->damage, explosion->pos, explosion->explosionRadius, MaskF::IsCorporealNotCreator, explosion);
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
			game->entities->TryDealDamageAll(puddle->damage, puddle->pos, puddle->radius, MaskF::IsCorporeal, puddle);
		}
		if (tTime - puddle->startTime > puddle->totalFadeTime)
			puddle->DestroySelf(puddle);
	}

	void VacuumeForU(Entity* entity)
	{
		VacuumeFor* vac = static_cast<VacuumeFor*>(entity);

		if (tTime - vac->startTime > vac->totalFadeTime)
		{
			vac->DestroySelf(nullptr);
			return;
		}

		game->entities->Vacuum(vac->pos, vac->vacDist, vac->vacSpeed, vac->maxVacSpeed, true);
	}
}

namespace DUpdates
{
	void FadeOutPuddleDU(Entity* entity)
	{
		FadeOutPuddle* puddle = static_cast<FadeOutPuddle*>(entity);
		puddle->color.a = 255 - static_cast<uint8_t>((tTime - puddle->startTime) * 255 / puddle->totalFadeTime);
		puddle->DUpdate(DUPDATE::ENTITY);
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

// Post entities definition items:

class PlacedOnLanding : public Item
{
public:
	bool sayCreator;
	Entity* entityToPlace;
	string creatorName;

	PlacedOnLanding(Entity* entityToPlace, string typeName, int intType = 0, int damage = 0, int count = 1, float range = 15.0f, bool sayCreator = false,
		float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(entityToPlace->name, typeName, intType, entityToPlace->color, damage, count, range, useTime, radius,
			corporeal, shouldCollide, mass, health), entityToPlace(entityToPlace), sayCreator(sayCreator)
	{
		itemOD = ITEMOD::PLACEDONLANDING;
	}

	PlacedOnLanding(Entity* entityToPlace, string name, string typeName, int intType = 0, RGBA color = RGBA(),
		int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false, float useTime = 0.25f, float radius = 0.5f,
		bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health),
		entityToPlace(entityToPlace), sayCreator(sayCreator)
	{
		itemOD = ITEMOD::PLACEDONLANDING;
	}

	PlacedOnLanding(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", string typeName = "NULL TYPE", int intType = 0,
		RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false,
		float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health),
		entityToPlace(entityToPlace), sayCreator(sayCreator)
	{
		itemOD = ITEMOD::PLACEDONLANDING;
	}

	Item Clone(int count) override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, intType, color, damage, count, range, sayCreator, useTime, radius, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2(int count) override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, intType, color, damage, count, range, sayCreator, useTime, radius, corporeal, shouldCollide, mass, health);
	}
};

class CorruptOnKill : public PlacedOnLanding
{
public:
	CorruptOnKill(Entity* entityToPlace, string typeName, int intType = 0, int damage = 0, int count = 1, float range = 15.0f, bool sayCreator = false,
		float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		PlacedOnLanding(entityToPlace, typeName, intType, damage, count, range, sayCreator,
			useTime, radius, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::CORRUPTONKILL;
	}

	CorruptOnKill(Entity* entityToPlace, string name, string typeName, int intType = 0, RGBA color = RGBA(),
		int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false, float useTime = 0.25f, float radius = 0.5f,
		bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		PlacedOnLanding(entityToPlace, name, typeName, intType, color, damage, count, range, sayCreator, useTime, radius, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::CORRUPTONKILL;
	}

	CorruptOnKill(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", string typeName = "NULL TYPE", int intType = 0,
		RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false,
		float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		PlacedOnLanding(baseClass, entityToPlace, name, typeName, intType, color, damage, count, range, sayCreator, useTime, radius, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::CORRUPTONKILL;
	}
};

inline void CreateExplosion(Vec3 pos, float explosionRadius, RGBA color, string name, int damage, int explosionDamage, Entity* creator)
{
	game->entities->push_back(make_unique<ExplodeNextFrame>(explosionDamage, explosionRadius, color, pos, name, creator));
	game->entities->push_back(make_unique<FadeOutGlow>(explosionRadius * 2.0f, static_cast<float>(explosionDamage + damage), pos, explosionRadius, color));
}

class ExplodeOnLanding : public Item
{
public:
	int explosionDamage;
	float explosionRadius;

	ExplodeOnLanding(float explosionRadius = 0.5f, int explosionDamage = 1, string name = "NULL", string typeName = "NULL TYPE", int intType = 0,
		RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, float useTime = 0.25f, float radius = 0.5f,
		bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health),
		explosionRadius(explosionRadius), explosionDamage(explosionDamage)
	{
		itemOD = ITEMOD::EXPLODEONLANDING;
	}

	ExplodeOnLanding(Item* baseClass, float explosionRadius = 0.5f, int explosionDamage = 1, string name = "NULL",
		string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f,
		float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health),
		explosionRadius(explosionRadius), explosionDamage(explosionDamage) { }

	Item Clone(int count) override
	{
		return ExplodeOnLanding(baseClass, explosionRadius, explosionDamage, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2(int count) override
	{
		return new ExplodeOnLanding(baseClass, explosionRadius, explosionDamage, name, typeName, intType, color, damage, count, range, useTime, radius);
	}
};

class ImproveSoilOnLanding : public Item
{
public:
	int improveRadius; // Used like a square's radius.
	ImproveSoilOnLanding(int improveRadius, string name = "NULL", string typeName = "NULL TYPE", int intType = 0,
		RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, float useTime = 0.25f, float radius = 0.5f,
		bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health),
		improveRadius(improveRadius)
	{
		itemOD = ITEMOD::IMPROVESOILONLANDING;
	}

	ImproveSoilOnLanding(int improveRadius, Item* baseClass, string name = "NULL",
		string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f,
		float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health),
		improveRadius(improveRadius) { }

	Item Clone(int count) override
	{
		return ImproveSoilOnLanding(improveRadius, baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2(int count) override
	{
		return new ImproveSoilOnLanding(improveRadius, baseClass, name, typeName, intType, color, damage, count, range, useTime, radius);
	}
};

class SetTileOnLanding : public Item
{
public:
	TILE tile;

	SetTileOnLanding(TILE tile, string name = "NULL", string typeName = "NULL TYPE", int intType = 0,
		RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, float useTime = 0.25f, float radius = 0.5f,
		bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health), tile(tile)
	{
		itemOD = ITEMOD::SETTILEONLANDING;
	}

	SetTileOnLanding(Item* baseClass, TILE tile, string name = "NULL",
		string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f,
		float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health),
		tile(tile) { }

	Item Clone(int count) override
	{
		return SetTileOnLanding(baseClass, tile, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2(int count) override
	{
		return new SetTileOnLanding(baseClass, tile, name, typeName, intType, color, damage, count, range, useTime, radius);
	}
};

namespace ItemODs
{
	void ItemOD(Item* item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		game->entities->push_back(make_unique<Collectible>(*item, pos));
	}

	void PlacedOnLandingOD(Item* item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		PlacedOnLanding* pOL = static_cast<PlacedOnLanding*>(item);

		if (((Entity*)game->player)->Overlaps(pos, pOL->radius))
		{
			pOL->OnDeath(ITEMOD::DEFAULT, pos, dir, creator, creatorName, callReason, callType);
			return;
		}
		unique_ptr<Entity> placedEntity = pOL->entityToPlace->Clone(pos, dir, creator);
		if (pOL->sayCreator)
			placedEntity->name += " from " + creatorName;
		game->entities->push_back(std::move(placedEntity));
	}

	void CorruptOnKillOD(Item* item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		item->OnDeath(callType == 2 ? ITEMOD::PLACEDONLANDING : ITEMOD::DEFAULT, pos, dir, creator, creatorName, callReason, callType);
	}

	void ExplodeOnLandingOD(Item* item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		ExplodeOnLanding* explosion = static_cast<ExplodeOnLanding*>(item);
		CreateExplosion(pos, explosion->explosionRadius, explosion->color, explosion->name + string(" shot by " + creatorName),
			explosion->damage, explosion->explosionDamage, creator);
	}

	void ImproveSoilOnLandingOD(Item* item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		ImproveSoilOnLanding* soilItem = static_cast<ImproveSoilOnLanding*>(item);

		for (int x = 0; x < soilItem->improveRadius; x++)
			for (int y = 0; y < soilItem->improveRadius; y++)
			{
				iVec3 currentPos = ToIV3(pos) + iVec3(x - soilItem->improveRadius / 2, y - soilItem->improveRadius / 2, 0);
				Chunk* chunk = game->entities->ChunkAtFPos(currentPos);
				TILE tile = TILE(chunk->TileAtPos(currentPos));
				chunk->SetTileAtPos(currentPos, UnEnum(tile == TILE::ROCK ? TILE::SAND : tile == TILE::SAND ? TILE::BAD_SOIL : tile == TILE::BAD_SOIL ?
					TILE::MID_SOIL : TILE::MAX_SOIL));
			}
	}

	void SetTileOnLandingOD(Item* item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		SetTileOnLanding* tileItem = static_cast<SetTileOnLanding*>(item);

		Chunk* chunk = game->entities->ChunkAtFPos(pos);
		chunk->SetTileAtPos(ToIV2(pos), UnEnum(tileItem->tile));
	}
}

namespace Hazards
{
	FadeOutPuddle* leadPuddle = new FadeOutPuddle(3.0f, 1, 0.2f, vZero, 1.5f, RGBA(80, 43, 92));
	VacuumeFor* vacuumPuddle = new VacuumeFor(vZero, 2, 5, -16, 16, RGBA(255, 255, 255, 51));
}

namespace Resources
{
	SetTileOnLanding* ruby = new SetTileOnLanding(TILE::RUBY_SOIL, "Ruby", "Tile", 5, RGBA(168, 50, 100), 0, 1, 15.f, 0.25f, 0.5f, false, false);
	ExplodeOnLanding* emerald = new ExplodeOnLanding(7.5f, 2, "Emerald", "Ammo", 1, RGBA(65, 224, 150), 2);
	ExplodeOnLanding* topaz = new ExplodeOnLanding(3.5f, 3, "Topaz", "Ammo", 1, RGBA(255, 200, 0), 0, 1, 15.0f, 0.25f, 1.5f);
	ExplodeOnLanding* sapphire = new ExplodeOnLanding(1.5f, 1, "Sapphire", "Ammo", 1, RGBA(78, 25, 212), 0, 1, 15.0f, 0.0625f);
	PlacedOnLanding* lead = new PlacedOnLanding(Hazards::leadPuddle, "Lead", "Deadly Ammo", 1, RGBA(80, 43, 92), 0, 1, 15.0f, true);
	PlacedOnLanding* vacuumium = new PlacedOnLanding(Hazards::vacuumPuddle, "Vacuumium", "Push Ammo", 1, RGBA(255, 255, 255), 0, 1, 15, false, 0.0625);
	ImproveSoilOnLanding* quartz = new ImproveSoilOnLanding(3, "Quartz", "Tile", 5, RGBA(156, 134, 194), 0, 1, 15, 0.125f, 0.5f, false, false);
}

namespace Collectibles
{
	Collectible* ruby = new Collectible(*Resources::ruby);
	Collectible* emerald = new Collectible(*Resources::emerald);
	Collectible* topaz = new Collectible(*Resources::topaz);
	Collectible* sapphire = new Collectible(*Resources::sapphire);
	Collectible* lead = new Collectible(*Resources::lead);
	Collectible* vacuumium = new Collectible(*Resources::vacuumium);
	Collectible* quartz = new Collectible(*Resources::quartz);
}