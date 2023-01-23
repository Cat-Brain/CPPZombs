#include "Collectible.h"
struct EntityIndex // For sorting.
{
	int index, valueForSorting;

	EntityIndex(int index = 0, int valueForSorting = 0) : index(index), valueForSorting(valueForSorting) {}

	bool operator < (const EntityIndex& other) const
	{
		return (valueForSorting < other.valueForSorting);
	}
};

#define CHUNK_WIDTH 8
#define MAP_WIDTH 50 // Defined in chunks not units.
#define MAP_WIDTH_TRUE CHUNK_WIDTH * MAP_WIDTH
class Chunk : public vector<int>
{
public:
	Vec2 pos;
	vector<byte> positions;

	Chunk(Vec2 pos = vZero) :
		vector({}), pos(pos), positions(0) { }

	~Chunk() { }

	bool Overlaps(Vec2 pos, Vec2 dimensions)
	{
		return pos.x - dimensions.x >= this->pos.x + dimensions.x && pos.x < this->pos.x + CHUNK_WIDTH &&
			pos.y - dimensions.y >= this->pos.y + dimensions.y && pos.y < this->pos.y + CHUNK_WIDTH;
	}

	void PrepareForRendering(vector<Entity*> entities)
	{
		positions.clear();
		positions.resize(CHUNK_WIDTH * CHUNK_WIDTH, 255);
		for (int i = 0; i < size(); i++) // There better not be > 255 
		{
			Entity* entity = entities[(*this)[i]];
			if (!entity->Corporeal())
				continue;
			Vec2 minPos = entity->pos - entity->dimensions + vOne - pos;
			Vec2 maxPos = entity->pos + entity->dimensions - vOne - pos;

			for (int x = max(0, minPos.x); x <= min(CHUNK_WIDTH - 1, maxPos.x); x++)
				for (int y = max(0, minPos.y); y <= min(CHUNK_WIDTH - 1, maxPos.y); y++)
					positions[x * CHUNK_WIDTH + y] = i;
		}
	}

	int IndexOfPosition(Vec2 pos) // Pos is in global space, also do NOT use if prepare for rendering has not been called.
	{
		if (!positions.size())
			return -1;
		Vec2 newPos = pos - this->pos;
		if (newPos.x >= 0 && newPos.y >= 0 && newPos.x < CHUNK_WIDTH && newPos.y < CHUNK_WIDTH && positions[newPos.x * CHUNK_WIDTH + newPos.y] != 255)
			return (*this)[positions[newPos.x * CHUNK_WIDTH + newPos.y]];
		return -1;
	}

	void UnprepareForRendering()
	{
		positions.clear();
	}
	
	static Vec2 ToSpace(Vec2 pos)
	{
		return Vec2(Clamp(pos.x / CHUNK_WIDTH, 0, MAP_WIDTH - 1), Clamp(pos.y / CHUNK_WIDTH, 0, MAP_WIDTH - 1));
	}

	static std::pair<Vec2, Vec2> MinMaxPos(Vec2 pos, Vec2 dimensions)
	{
		return { ToSpace(pos - dimensions), ToSpace(pos + dimensions) };
	}
};

class LightSource;
class Entities : public vector<Entity*>
{
protected:
	int index;

public:
	bool addedEntity;
	vector<Entity*> sortedNCEntities; // The NC stands for Non-Collectible.
	vector<Entity*> collectibles; // sortedNCEntities and collectibles are the most accurate, the others are less so.
	vector<LightSource*> lightSources;
	vector<Particle*> particles;
	Chunk chunks[MAP_WIDTH][MAP_WIDTH];
	vector<Chunk*> renderedChunks;

	Entities() :
		vector(0), addedEntity(false), index(0)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
			for (int y = 0; y < MAP_WIDTH; y++)
				chunks[x][y] = Chunk({ x * CHUNK_WIDTH, y * CHUNK_WIDTH });
	}

	void push_back(Entity* entity)
	{
		entity->pos.x = Clamp(entity->pos.x, entity->dimensions.x - 1, MAP_WIDTH_TRUE - entity->dimensions.x + 1);
		entity->pos.y = Clamp(entity->pos.y, entity->dimensions.y - 1, MAP_WIDTH_TRUE - entity->dimensions.y + 1);

		addedEntity = true;
		vector<Entity*>::push_back(entity);

		if (entity->IsCollectible())
			collectibles.push_back(entity);
		else
		{
			index++;
			sortedNCEntities.insert(sortedNCEntities.begin(), entity);
		}

		vector<Chunk*> chunkOverlaps = ChunkOverlaps(entity->pos, entity->dimensions);
		for (Chunk* chunk : chunkOverlaps)
			chunk->push_back(static_cast<int>(size() - 1));
	}

	Chunk* ChunkAtPos(Vec2 pos)
	{
		Vec2 chunkPos = Chunk::ToSpace(pos);
		return &chunks[chunkPos.x][chunkPos.y];
	}

	vector<Chunk*> MainChunkOverlaps(Vec2 minPos, Vec2 maxPos) // In chunk coords, do NOT plug in normal space coords.
	{
		vector<Chunk*> result((maxPos.x - minPos.x + 1) * (maxPos.y - minPos.y + 1));
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				result[i++] = &chunks[x][y];
		return result;
	}

	vector<Chunk*> ChunkOverlaps(Vec2 pos, Vec2 dimensions)
	{
		std::pair<Vec2, Vec2> minMaxPos = Chunk::MinMaxPos(pos, dimensions);
		return MainChunkOverlaps(minMaxPos.first, minMaxPos.second);
	}

	Entity* FindNearestEnemy(Vec2 pos, Vec2 farthestDimensions)
	{
		vector<Entity*> nearbyEntities = FindCorpOverlaps(pos, farthestDimensions);
		float currentBestDist = 9999.0f;
		Entity* currentBest = nullptr;
		for (Entity* entity : nearbyEntities)
		{
			float dist;
			if (entity->IsEnemy() && (dist = Distance(pos, entity->pos)) < currentBestDist)
			{
				currentBestDist = dist;
				currentBest = entity;
			}
		}
		return currentBest;
	}

	vector<Entity*> FindCorpOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 hDim)
	{
		vector<Entity*> overlaps{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*this)[*iter]->Corporeal() && (*this)[*iter]->Overlaps(pos, hDim) &&
					(find(overlaps.begin(), overlaps.end(), (*this)[*iter]) == overlaps.end())) overlaps.push_back((*this)[*iter]);
		return overlaps;
	}

	vector<Entity*> FindCorpOverlaps(Vec2 pos, Vec2 hDim)
	{
		return FindCorpOverlaps(ChunkOverlaps(pos, hDim), pos, hDim);
	}

	vector<Entity*> FindIncorpOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 hDim)
	{
		vector<Entity*> overlaps{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*this)[*iter]->Corporeal() && (*this)[*iter]->Overlaps(pos, hDim)) overlaps.push_back((*this)[*iter]);
		return overlaps;
	}

	vector<Entity*> FindIncorpOverlaps(Vec2 pos, Vec2 hDim)
	{
		return FindIncorpOverlaps(ChunkOverlaps(pos, hDim), pos, hDim);
	}

	vector<Entity*> FindAllOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 hDim)
	{
		vector<Entity*> overlaps{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*this)[*iter]->Overlaps(pos, hDim) &&
					((*this)[*iter]->dimensions == vOne || find(overlaps.begin(), overlaps.end(), (*this)[*iter]) == overlaps.end())) overlaps.push_back((*this)[*iter]);
		return overlaps;
	}

	vector<Entity*> FindAllOverlaps(Vec2 pos, Vec2 hDim)
	{
		return FindAllOverlaps(ChunkOverlaps(pos, hDim), pos, hDim);
	}

	std::pair<vector<Entity*>, vector<Entity*>> FindPairOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 hDim) // Returns {corporeals, incorporeals}
	{
		vector<Entity*> corporeals{}, incorporeals{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*this)[*iter]->Overlaps(pos, hDim) &&
					((*this)[*iter]->dimensions == vOne ||
						((*this)[*iter]->Corporeal() && find(corporeals.begin(), corporeals.end(), (*this)[*iter]) == corporeals.end()) ||
						(!(*this)[*iter]->Corporeal() && find(incorporeals.begin(), incorporeals.end(), (*this)[*iter]) == incorporeals.end())))
				{
					if ((*this)[*iter]->Corporeal())
						corporeals.push_back((*this)[*iter]);
					else
						incorporeals.push_back((*this)[*iter]);
				}
		return { corporeals, incorporeals };
	}

	std::pair<vector<Entity*>, vector<Entity*>> FindPairOverlaps(Vec2 pos, Vec2 hDim)
	{
		return FindPairOverlaps(ChunkOverlaps(pos, hDim), pos, hDim);
	}

	vector<Entity*> RayTraceIntersections(Vec2 startPos, Vec2 endPos)
	{
		vector<Entity*> overlaps(0);

		Vec2 start = Chunk::ToSpace(startPos), end = Chunk::ToSpace(endPos);
		Vec2 absDelta = Vabs(end - start);
		Vec2 currentPos = startPos;
		int n = 1 + absDelta.x + absDelta.y;
		Vec2 inc = Vec2(int(end.x > start.x) * 2 - 1, int(end.y > start.y) * 2 - 1);
		int error = absDelta.x - absDelta.y;
		absDelta *= 2;

		for (; n > 0; --n)
		{
#pragma region Raytrace on the chunk



#pragma endregion

			if (error > 0)
			{
				currentPos.x += inc.x;
				error -= absDelta.y;
			}
			else
			{
				currentPos.y += inc.y;
				error += absDelta.x;
			}
		}
	}

	// Add more overlap functions.

	void SortEntities()
	{
		int length = static_cast<int>(size());
		for (Entity* entity : *this)
			length -= int(entity->IsCollectible());
		vector<EntityIndex> unsortedToSorted = vector<EntityIndex>(length);
		for (int i = 0, j = 0; i < size(); i++)
			if (!(*this)[i]->IsCollectible())
			{
				unsortedToSorted[j] = EntityIndex(i, (*this)[i]->SortOrder());
				j++;
			}
		std::sort(unsortedToSorted.begin(), unsortedToSorted.end());

		sortedNCEntities = vector<Entity*>(length);
		for (int i = 0, j = 0; i < size(); i++)
			if (!(*this)[i]->IsCollectible())
			{
				sortedNCEntities[j] = (*this)[unsortedToSorted[j].index];
				j++;
			}

		collectibles = vector<Entity*>(size() - length);
		length = 0;
		for (Entity* entity : *this)
			if (entity->IsCollectible())
				collectibles[length++] = entity;
		addedEntity = false;
	}

	void Update()
	{
		if (addedEntity)
			SortEntities();

		for (int i = 0; i < collectibles.size(); i++)
			if (collectibles[i]->active)
				collectibles[i]->Update();

		if (addedEntity)
			SortEntities();

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->active)
				sortedNCEntities[index]->Update();

		if (addedEntity)
			SortEntities();

		for (int i = 0; i < particles.size(); i++)
		{
			particles[i]->Update();
			if (particles[i]->ShouldEnd())
			{
				Particle* toDestroyParticle = particles[i];
				particles.erase(particles.begin() + i);
				delete toDestroyParticle;
				i--;
			}
		}
	}

	void DUpdate()
	{
		std::pair<vector<Entity*>, vector<Entity*>> toRenderPair = FindPairOverlaps(playerPos, screenDimH + vOne); // Collectibles then NCs.
		// Early pass
		for (Entity* entity : toRenderPair.second)
			entity->EarlyDUpdate();
		for (Entity* entity : toRenderPair.first)
			entity->EarlyDUpdate();
		// Normal pass
		for (Entity* entity : toRenderPair.second)
			entity->DUpdate();
		for (Entity* entity : toRenderPair.first)
			entity->DUpdate();

		for (Particle* particle : particles)
			particle->LowResUpdate();

		// Double the size that would normally be rendered, this is just for doing lighting, so it needs to go a bit further.
		vector<Chunk*> newRenderedChunks = MainChunkOverlaps(Chunk::ToSpace(playerPos - screenDim), Chunk::ToSpace(playerPos + screenDim));
		if (renderedChunks != newRenderedChunks)
			for (Chunk* chunk : renderedChunks)
				chunk->UnprepareForRendering();
		renderedChunks = newRenderedChunks;
		for (Chunk* chunk : renderedChunks)
			chunk->PrepareForRendering(*this);
	}

	void UIUpdate()
	{
		for (Particle* particle : particles)
			particle->HighResUpdate();

		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->dActive && collectibles[index]->shouldUI)
				collectibles[index]->UIUpdate();

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->dActive && sortedNCEntities[index]->shouldUI)
				sortedNCEntities[index]->UIUpdate();
	}

	void Remove(Entity* entityToRemove)
	{
		// Remove from sortedNCEntities or from collectibles.
		if (!entityToRemove->IsCollectible())
		{
			vector<Entity*>::iterator pos = find(sortedNCEntities.begin(), sortedNCEntities.end(), entityToRemove);
			index -= int(index >= distance(sortedNCEntities.begin(), pos)); // If index is past or at the position being removed then don't advance.
			sortedNCEntities.erase(pos);
		}
		else
			collectibles.erase(find(collectibles.begin(), collectibles.end(), entityToRemove));
		// Remove from every chunk that this object overlaps.
		vector<Entity*>::iterator mainPos = find(begin(), end(), entityToRemove);
		int removalIndex = static_cast<int>(distance(begin(), mainPos));
		vector<Chunk*> chunkOverlaps = ChunkOverlaps(entityToRemove->pos, entityToRemove->dimensions);
		for (Chunk* chunk : chunkOverlaps)
			chunk->erase(find(chunk->begin(), chunk->end(), removalIndex));

		for (int x = 0; x < MAP_WIDTH; x++)
			for (int y = 0; y < MAP_WIDTH; y++)
				for (int i = 0; i < chunks[x][y].size(); i++)
					chunks[x][y][i] -= int(chunks[x][y][i] > removalIndex);

		// Remove from main list from which the rest derive.
		erase(mainPos);
		
	}

	void Remove(LightSource* lightSourceToRemove)
	{
		lightSources.erase(find(lightSources.begin(), lightSources.end(), lightSourceToRemove));
	}

	void Vacuum(Vec2 pos, int vacDist)
	{
		for (Entity* collectible : collectibles)
		{
			int distance = Diagnistance(pos, collectible->pos);
			if (collectible->active && distance > 0 && distance <= vacDist)
			{
				collectible->SetPos(collectible->pos + Squarmalized(pos - collectible->pos));
			}
		}
	}

	void VacuumCone(Vec2 pos, Vec2 dir, int vacDist, float fov)
	{
		for (Entity* collectible : collectibles)
		{
			int distance = Diagnistance(pos, collectible->pos);
			if (collectible->active && distance > 0 && distance <= vacDist && Dot(dir, Normalized(collectible->pos - pos + dir)) >= 1 - fov)
			{
				collectible->SetPos(collectible->pos + Squarmalized(pos - collectible->pos));
			}
		}
	}
};

#pragma region Post Entities functions

int Entity::DealDamage(int damage, Entity* damageDealer)
{
	if (damage > 0)
		game->entities->particles.push_back(new SpinText(pos + Vec2f(RandFloat(), RandFloat()) * (2 * dimensions - vOne) - up, damage, to_string(damage),
			Color(damageDealer->color.r, damageDealer->color.g, damageDealer->color.b, damageDealer->color.a / 2),
			0.5f, RandFloat() * 5.0f, RandFloat() * 0.125f + 0.125f));
	
	health -= damage;
	if (health <= 0)
	{
		DestroySelf(damageDealer);
		return 1;
	}
	return 0;
}

void Item::OnDeath(Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType)
{
	game->entities->push_back(new Collectible(*this, pos));
}

void Entity::DestroySelf(Entity* damageDealer)
{
	if (shouldUI)
		game->MenuedEntityDied(this);
	game->entities->Remove(this);
	OnDeath(damageDealer);
	if (holder != nullptr)
		holder->heldEntity = nullptr;
	delete this;
}

bool Entity::TryMove(Vec2 direction, int force, Entity* ignore, Entity** hitEntity) // returns index of hit item.
{
	Vec2 newPos = ClampV2(pos + direction, vZero, vOne * (MAP_WIDTH_TRUE - 1));
	vector<Chunk*> chunkOverlaps;
	if (force >= mass && direction != Vec2(0, 0))
	{
		chunkOverlaps = game->entities->ChunkOverlaps(newPos, dimensions);
		vector<Entity*> overlaps = game->entities->FindCorpOverlaps(chunkOverlaps, newPos, dimensions);
		for (Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != entity->creator || creator == nullptr))
			{
				if (hitEntity != nullptr)
					*hitEntity = entity;
				if (!entity->TryMove(direction, force - mass, ignore) && !entity->Overlaps(pos, dimensions))
				{
					// something in front of them, however if they're stuck, we want to let them move anyways.
					vector<Entity*> overlaps2 = game->entities->FindCorpOverlaps(pos, dimensions);
					bool successful = false;
					for (Entity* entity2 : overlaps2)
						if (entity2 != ignore && entity2 != this && (creator != entity2->creator || creator == nullptr) &&
							force - mass > entity2->mass)
						{
							successful = true;
							break;
						}
					if (successful)
						break;
					return false; // The entity is not stuck inside another entity and are blocked.
				}
			}
	}
	else return false;

	SetPos(newPos);

	return true;
}

void Entity::SetPos(Vec2 newPos)
{
	Vec2 clampedNewPos = ClampV2(newPos, vZero, vOne * (MAP_WIDTH_TRUE - 1));
	std::pair<Vec2, Vec2> minMaxOldPos = Chunk::MinMaxPos(pos, dimensions);
	std::pair<Vec2, Vec2> minMaxNewPos = Chunk::MinMaxPos(clampedNewPos, dimensions);
	if (minMaxOldPos != minMaxNewPos)
	{
		int position = static_cast<int>(distance(game->entities->begin(), find(game->entities->begin(), game->entities->end(), this)));
		vector<Chunk*> oldChunkOverlaps = game->entities->MainChunkOverlaps(minMaxOldPos.first, minMaxOldPos.second);
		for (Chunk* chunk : oldChunkOverlaps)
			chunk->erase(find(chunk->begin(), chunk->end(), position));
		vector<Chunk*> newChunkOverlaps = game->entities->MainChunkOverlaps(minMaxNewPos.first, minMaxNewPos.second);
		for (Chunk* chunk : newChunkOverlaps)
			chunk->push_back(position);
	}
	pos = clampedNewPos;
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
	Vec2 explosionDimensions;
	float startTime;

	ExplodeNextFrame(int damage = 1, Vec2 explosionDimensions = vOne, Color color = olc::WHITE , Vec2 pos = vZero, string name = "NULL NAME", Entity* creator = nullptr) :
		Entity(pos, vOne, color, color, 1, 1, 1, string("Explosion from ") + name), damage(damage), explosionDimensions(explosionDimensions), startTime(tTime)
	{
		this->creator = creator;
	}


	void Update() override
	{
		if (tTime != startTime)
		{
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(pos, explosionDimensions);
			for (Entity* entity : hitEntities)
				if (entity != this && entity != creator)
					entity->DealDamage(damage, this);
			for (int i = 0; i < EXPLOSION_PARTICLE_COUNT; i++)
			{
				float rotation = RandFloat() * PI_F * 2;
				game->entities->particles.push_back(new VelocitySquare(pos, Vec2f(sinf(rotation), cosf(rotation)) * EXPLOSION_PARTICLE_SPEED,
					color, EXPLOSION_PARTICLE_DURATION));
			}
			DestroySelf(this);
		}
	}

	bool Corporeal() override
	{
		return false;
	}
};

class FadeOutPuddle : public Entity
{
public:
	int damage;
	float startTime, totalFadeTime, timePer, lastTime;

	FadeOutPuddle(float totalFadeTime = 1.0f, int damage = 1, float timePer = 1.0f, Vec2 pos = Vec2(0, 0),
		Vec2 dimensions = Vec2(1, 1), Color color = Color(olc::WHITE)) :
		Entity(pos, dimensions, color, color, 1, 1, 1, "Puddle"),
		totalFadeTime(totalFadeTime), damage(damage), startTime(tTime), timePer(timePer), lastTime(tTime) { }

	FadeOutPuddle(FadeOutPuddle* baseClass, Vec2 pos) :
		FadeOutPuddle(*baseClass) {
		this->pos = pos;
		startTime = tTime;
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		FadeOutPuddle* newPuddle = new FadeOutPuddle(this, pos);
		return newPuddle;
	}

	void Update() override
	{
		if (tTime - lastTime > timePer)
		{
			lastTime = tTime;
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(pos, dimensions);
			for (Entity* entity : hitEntities)
				entity->DealDamage(damage, this);
		}
		if (tTime - startTime > totalFadeTime)
			DestroySelf(nullptr);
	}

	void DUpdate() override
	{
		color.a = 255 - static_cast<uint8_t>((tTime - startTime) * 255 / totalFadeTime);
		Entity::DUpdate();
	}

	bool Corporeal() override
	{
		return false;
	}
};

// Post entities definition items:

class PlacedOnLanding : public Item
{
public:
	bool sayCreator;
	Entity* entityToPlace;
	string creatorName;

	PlacedOnLanding(Entity* entityToPlace, string typeName, int damage = 0, int count = 1, float range = 15.0f, bool sayCreator = false, Vec2 dimensions = vOne) :
		Item(entityToPlace->name, typeName, entityToPlace->color, damage, count, range, dimensions), entityToPlace(entityToPlace), sayCreator(sayCreator){ }

	PlacedOnLanding(Entity* entityToPlace, string name, string typeName, Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false, Vec2 dimensions = vOne) :
		Item(name, typeName, color, damage, count, range, dimensions), entityToPlace(entityToPlace), sayCreator(sayCreator) { }

	PlacedOnLanding(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", string typeName = "NULL TYPE", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false, Vec2 dimensions = vOne) :
		Item(baseClass, name, typeName, color, damage, count, range, dimensions), entityToPlace(entityToPlace), sayCreator(sayCreator) { }

	Item Clone(int count) override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, sayCreator, dimensions);
	}

	Item Clone() override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, sayCreator, dimensions);
	}

	Item* Clone2(int count) override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, sayCreator, dimensions);
	}

	Item* Clone2() override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, sayCreator, dimensions);
	}

	void OnDeath(Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType) override
	{
		Entity* placedEntity = entityToPlace->Clone(pos, up, creator);
		if (sayCreator)
			placedEntity->name += " from " + creatorName;
		game->entities->push_back(placedEntity);
	}
};

class ExplodeOnLanding : public Item
{
public:
	Vec2 explosionDimensions;

	ExplodeOnLanding(Vec2 explosionDimensions = vOne, string name = "NULL", string typeName = "NULL TYPE", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f, Vec2 dimensions = vOne) :
		Item(name, typeName, color, damage, count, range, dimensions), explosionDimensions(explosionDimensions) { }

	ExplodeOnLanding(Item* baseClass, Vec2 explosionDimensions = vOne, string name = "NULL", string typeName = "NULL TYPE", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f, Vec2 dimensions = vOne) :
		Item(baseClass, name, typeName, color, damage, count, range, dimensions), explosionDimensions(explosionDimensions) { }

	virtual Item Clone(int count)
	{
		return ExplodeOnLanding(baseClass, explosionDimensions, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item Clone()
	{
		return ExplodeOnLanding(baseClass, explosionDimensions, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item* Clone2(int count)
	{
		return new ExplodeOnLanding(baseClass, explosionDimensions, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item* Clone2()
	{
		return new ExplodeOnLanding(baseClass, explosionDimensions, name, typeName, color, damage, count, range, dimensions);
	}

	void OnDeath(Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType) override
	{
		game->entities->push_back(new ExplodeNextFrame(damage, explosionDimensions, color, pos, name + string(" shot by ") + creatorName, creator));
		game->entities->push_back(new FadeOut(0.5f, pos, explosionDimensions, color));
	}
};

class CorruptOnKill : public PlacedOnLanding
{
public:
	using PlacedOnLanding::PlacedOnLanding;

	void OnDeath(Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType) override
	{
		if (callType == 2)
			PlacedOnLanding::OnDeath(pos, creator, creatorName, callReason, callType);
		else
			Item::OnDeath(pos, creator, creatorName, callReason, callType);
	}
};

namespace Hazards
{
	FadeOutPuddle* leadPuddle = new FadeOutPuddle(3.0f, 1, 0.2f, vZero, vOne * 2, Color(80, 43, 92));
}

namespace Resources
{
	ExplodeOnLanding* ruby = new ExplodeOnLanding(vOne * 3, "Ruby", "Ammo", Color(168, 50, 100), 4);
	ExplodeOnLanding* emerald = new ExplodeOnLanding(vOne * 8, "Emerald", "Ammo", Color(65, 224, 150), 2);
	ExplodeOnLanding* topaz = new ExplodeOnLanding(vOne * 4, "Topaz", "Ammo", Color(255, 200, 0), 3, 1, 15.0f, vOne * 2);
	PlacedOnLanding* lead = new PlacedOnLanding(Hazards::leadPuddle, "Lead", "Ammo", Color(80, 43, 92), 0, 1, 15.0f, true);
}

namespace Collectibles
{
	Collectible* ruby = new Collectible(*Resources::ruby, vZero);
	Collectible* emerald = new Collectible(*Resources::emerald, vZero);
	Collectible* topaz = new Collectible(*Resources::topaz, vZero);
	Collectible* lead = new Collectible(*Resources::lead, vZero);
}