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

	void PrepareForRendering(vector<unique_ptr<Entity>>* entities)
	{/*
		positions.clear();
		positions.resize(CHUNK_WIDTH * CHUNK_WIDTH, 255);
		for (int i = 0; i < size(); i++) // There better not be > 255 
		{
			Entity* entity = (*entities)[(*this)[i]].get();
			if (!entity->Corporeal())
				continue;

			iVec2 minPos = entity->pos - entity->dimensions + vOne - pos;
			iVec2 maxPos = entity->pos + entity->dimensions - vOne - pos;

			for (int x = max(0, minPos.x); x <= min(CHUNK_WIDTH - 1, maxPos.x); x++)
				for (int y = max(0, minPos.y); y <= min(CHUNK_WIDTH - 1, maxPos.y); y++)
					positions[x * CHUNK_WIDTH + y] = i;
		}*/
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
	
	static Vec2 ToSpace(Vec2f pos)
	{
		return Vec2(Clamp(pos.x / CHUNK_WIDTH, 0, MAP_WIDTH - 1), Clamp(pos.y / CHUNK_WIDTH, 0, MAP_WIDTH - 1));
	}

	static std::pair<Vec2, Vec2> MinMaxPos(Vec2 pos, Vec2 dimensions)
	{
		return { ToSpace(pos - dimensions / 2), ToSpace((pos + dimensions / 2)) };
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
	vector<unique_ptr<Particle>> particles;
	Chunk chunks[MAP_WIDTH][MAP_WIDTH];
	vector<Chunk*> renderedChunks;

	Entities() :
		vector(0), addedEntity(false), index(0)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
			for (int y = 0; y < MAP_WIDTH; y++)
				chunks[x][y] = Chunk({ x * CHUNK_WIDTH, y * CHUNK_WIDTH });
	}

	void push_back(unique_ptr<Entity> entity)
	{
		entity->pos.x = Clamp(entity->pos.x, entity->dimensions.x - 1, MAP_WIDTH_TRUE - entity->dimensions.x + 1);
		entity->pos.y = Clamp(entity->pos.y, entity->dimensions.y - 1, MAP_WIDTH_TRUE - entity->dimensions.y + 1);

		addedEntity = true;
		if (entity->IsCollectible())
			collectibles.push_back(entity.get());
		else
		{
			index++;
			sortedNCEntities.insert(sortedNCEntities.begin(), entity.get());
		}

		vector<Chunk*> chunkOverlaps = ChunkOverlaps(entity->pos, entity->dimensions);
		for (Chunk* chunk : chunkOverlaps)
			chunk->push_back(static_cast<int>(size()));

		vector<unique_ptr<Entity>>::push_back(std::move(entity));
	}

#pragma region Overlaps and collisionstuff
	Chunk* ChunkAtPos(Vec2f pos)
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
		float currentBestDist = 9999.0f; // Sqr magnitude not real magnitude.
		Entity* currentBest = nullptr;
		for (Entity* entity : nearbyEntities)
		{
			float dist;
			if (entity->IsEnemy() && (dist = Vec2f(pos - entity->pos).SqrMagnitude()) < currentBestDist)
			{
				currentBestDist = dist;
				currentBest = entity;
			}
		}
		return currentBest;
	}

	vector<Entity*> FindCorpOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 dim)
	{
		vector<Entity*> overlaps{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*this)[*iter]->Corporeal() && (*this)[*iter]->Overlaps(pos, dim) &&
					find(overlaps.begin(), overlaps.end(), (*this)[*iter].get()) == overlaps.end()) overlaps.push_back((*this)[*iter].get());
		return overlaps;
	}

	vector<Entity*> FindCorpOverlaps(Vec2 pos, Vec2 dim)
	{
		return FindCorpOverlaps(ChunkOverlaps(pos, dim), pos, dim);
	}

	vector<Entity*> FindIncorpOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 dim)
	{
		vector<Entity*> overlaps{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*this)[*iter]->Corporeal() && (*this)[*iter]->Overlaps(pos, dim)) overlaps.push_back((*this)[*iter].get());
		return overlaps;
	}

	vector<Entity*> FindIncorpOverlaps(Vec2 pos, Vec2 dim)
	{
		return FindIncorpOverlaps(ChunkOverlaps(pos, dim), pos, dim);
	}

	vector<Entity*> FindAllOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 dim)
	{
		vector<Entity*> overlaps{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*this)[*iter]->Overlaps(pos, dim) &&
					find(overlaps.begin(), overlaps.end(), (*this)[*iter].get()) == overlaps.end()) overlaps.push_back((*this)[*iter].get());
		return overlaps;
	}

	vector<Entity*> FindAllOverlaps(Vec2 pos, Vec2 dim)
	{
		return FindAllOverlaps(ChunkOverlaps(pos, dim), pos, dim);
	}

	std::pair<vector<Entity*>, vector<Entity*>> FindPairOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 dim) // Returns {corporeals, incorporeals}
	{
		vector<Entity*> corporeals{}, incorporeals{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*this)[*iter]->Overlaps(pos, dim) &&
					(((*this)[*iter]->Corporeal() && find(corporeals.begin(), corporeals.end(), (*this)[*iter].get()) == corporeals.end()) ||
						(!(*this)[*iter]->Corporeal() && find(incorporeals.begin(), incorporeals.end(), (*this)[*iter].get()) == incorporeals.end())))
				{
					if ((*this)[*iter]->Corporeal())
						corporeals.push_back((*this)[*iter].get());
					else
						incorporeals.push_back((*this)[*iter].get());
				}
		return { corporeals, incorporeals };
	}

	std::pair<vector<Entity*>, vector<Entity*>> FindPairOverlaps(Vec2 pos, Vec2 dim)
	{
		return FindPairOverlaps(ChunkOverlaps(pos, dim), pos, dim);
	}

	vector<Entity*> RayTraceIntersections(Vec2 startPos, Vec2 endPos)
	{
		vector<Entity*> overlaps(0);

		Vec2f start = Chunk::ToSpace(startPos), end = Chunk::ToSpace(endPos);
		Vec2f absDelta = (end - start).Abs();
		Vec2f currentPos = startPos;
		int n = 1 + absDelta.x + absDelta.y;
		Vec2f inc = Vec2f(int(end.x > start.x) * 2 - 1, int(end.y > start.y) * 2 - 1);
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
#pragma endregion

	void SortEntities()
	{
		int length = static_cast<int>(size());
		for (unique_ptr<Entity>& entity : *this)
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
				sortedNCEntities[j] = (*this)[unsortedToSorted[j].index].get();
				j++;
			}

		collectibles = vector<Entity*>(size() - length);
		length = 0;
		for (unique_ptr<Entity>& entity : *this)
			if (entity->IsCollectible())
				collectibles[length++] = entity.get();
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
				Particle* toDestroyParticle = particles[i].get();
				particles.erase(particles.begin() + i);
				i--;
			}
		}
	}

	void DUpdate()
	{
		std::pair<vector<Entity*>, vector<Entity*>> toRenderPair = FindPairOverlaps(game->PlayerPos(), ScrDim() + vOne); // Collectibles then NCs.
		// Collectibles
		for (Entity* entity : toRenderPair.second)
			entity->EarlyDUpdate();
		for (Entity* entity : toRenderPair.second)
			entity->DUpdate();
		// Normal entities
		for (Entity* entity : toRenderPair.first)
			entity->EarlyDUpdate();
		for (Entity* entity : toRenderPair.first)
			entity->DUpdate();

		for (unique_ptr<Particle>& particle : particles)
			particle->LowResUpdate();

		// Double the size that would normally be rendered, this is just for doing lighting, so it needs to go a bit further.
		vector<Chunk*> newRenderedChunks = MainChunkOverlaps(Chunk::ToSpace(game->PlayerPos() - ScrDim()), Chunk::ToSpace(game->PlayerPos() + ScrDim()));
		if (renderedChunks != newRenderedChunks)
			for (Chunk* chunk : renderedChunks)
				chunk->UnprepareForRendering();
		renderedChunks = newRenderedChunks;
		for (Chunk* chunk : renderedChunks)
			chunk->PrepareForRendering(this);
	}

	void UIUpdate()
	{
		for (unique_ptr<Particle>& particle : particles)
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
			vector<Entity*>::iterator pos = std::find(sortedNCEntities.begin(), sortedNCEntities.end(), entityToRemove);
			index -= int(index >= distance(sortedNCEntities.begin(), pos)); // If index is past or at the position being removed then don't advance.
			sortedNCEntities.erase(pos);
		}
		else
			collectibles.erase(std::find(collectibles.begin(), collectibles.end(), entityToRemove));
		// Remove from every chunk that this object overlaps.
		vector<unique_ptr<Entity>>::iterator mainPos = std::find_if(begin(), end(), [entityToRemove](std::unique_ptr<Entity> const& i) { return i.get() == entityToRemove; });;
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
		lightSources.erase(std::find_if(lightSources.begin(), lightSources.end(), [lightSourceToRemove](std::unique_ptr<LightSource> const& i) { return i.get() == lightSourceToRemove; }));
	}

	void Remove(Particle* particleToRemove)
	{
		particles.erase(std::find_if(particles.begin(), particles.end(), [particleToRemove](std::unique_ptr<Particle> const& i) { return i.get() == particleToRemove; }));
	}

	void Vacuum(Vec2 pos, int vacDist)
	{
		for (Entity* collectible : collectibles)
		{
			int distance = pos.Squistance(collectible->pos);
			if (collectible->active && distance > 0 && distance <= vacDist)
			{
				collectible->SetPos(collectible->pos + Vec2f(pos - collectible->pos).Rormalized());
			}
		}
	}

	void VacuumCone(Vec2f pos, Vec2f dir, int vacDist, float fov)
	{
		for (Entity* collectible : collectibles)
		{
			int distance = pos.Distance(collectible->pos);
			if (collectible->active && distance > 0 && distance <= vacDist && dir.Dot(Vec2f(collectible->pos - pos + dir).Normalized()) >= 1 - fov)
			{
				collectible->SetPos(collectible->pos + Vec2f(pos - collectible->pos).Rormalized());
			}
		}
	}
};

#pragma region Post Entities functions

int Entity::DealDamage(int damage, Entity* damageDealer)
{
	if (damage > 0)
		game->entities->particles.push_back(make_unique<SpinText>(pos + Vec2f(RandFloat(), RandFloat()) * (dimensions * 2 - vOne) - up, damage, to_string(damage),
			RGBA(damageDealer->color.r, damageDealer->color.g, damageDealer->color.b, damageDealer->color.a / 2),
			0.5f, RandFloat() * 5.0f, RandFloat() * 0.125f + 0.125f));
	
	health -= damage;
	if (health <= 0)
	{
		DestroySelf(damageDealer);
		return 1;
	}
	return 0;
}

void Item::OnDeath(Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType)
{
	game->entities->push_back(make_unique<Collectible>(*this, pos));
}

void Entity::DestroySelf(Entity* damageDealer)
{
	if (shouldUI)
		game->MenuedEntityDied(this);
	OnDeath(damageDealer);
	if (holder != nullptr)
		holder->heldEntity = nullptr;
	game->entities->Remove(this);
}

bool Entity::TryMove(Vec2 direction, int force, Entity* ignore, Entity** hitEntity) // returns index of hit item.
{
	Vec2 newPos = (pos + direction).ClampV(vZero, vOne * (MAP_WIDTH_TRUE - 1));
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
	Vec2 clampedNewPos = newPos.ClampV(dimensions, vOne * (MAP_WIDTH_TRUE - 1) - dimensions);
	std::pair<Vec2, Vec2> minMaxOldPos = Chunk::MinMaxPos(pos, dimensions);
	std::pair<Vec2, Vec2> minMaxNewPos = Chunk::MinMaxPos(clampedNewPos, dimensions);
	if (minMaxOldPos.first != minMaxNewPos.first || minMaxOldPos.second != minMaxNewPos.second)
	{
		int position = static_cast<int>(distance(game->entities->begin(), std::find_if(game->entities->begin(), game->entities->end(), [this](std::unique_ptr<Entity> const& i) { return i.get() == this; })));
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
	Vec2f explosionDimensions;
	float startTime;

	ExplodeNextFrame(int damage = 1, Vec2 explosionDimensions = vOne, RGBA color = RGBA(), Vec2 pos = vZero, string name = "NULL NAME", Entity* creator = nullptr) :
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
				game->entities->particles.push_back(make_unique<VelocitySquare>(pos, Vec2f(sinf(rotation), cosf(rotation)) * EXPLOSION_PARTICLE_SPEED,
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

	FadeOutPuddle(float totalFadeTime = 1.0f, int damage = 1, float timePer = 1.0f, Vec2f pos = Vec2f(0, 0),
		Vec2f dimensions = Vec2f(1, 1), RGBA color = RGBA()) :
		Entity(pos, dimensions, color, color, 1, 1, 1, "Puddle"),
		totalFadeTime(totalFadeTime), damage(damage), startTime(tTime), timePer(timePer), lastTime(tTime) { }

	FadeOutPuddle(FadeOutPuddle* baseClass, Vec2f pos) :
		FadeOutPuddle(*baseClass) {
		this->pos = pos;
		startTime = tTime;
	}

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<FadeOutPuddle>(this, pos);
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

	PlacedOnLanding(Entity* entityToPlace, string typeName, int damage = 0, int count = 1, float range = 15.0f, bool sayCreator = false, float shootSpeed = 0.25f, Vec2f dimensions = vOne) :
		Item(entityToPlace->name, typeName, entityToPlace->color, damage, count, range, shootSpeed, dimensions), entityToPlace(entityToPlace), sayCreator(sayCreator){ }

	PlacedOnLanding(Entity* entityToPlace, string name, string typeName, RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false, float shootSpeed = 0.25f, Vec2f dimensions = vOne) :
		Item(name, typeName, color, damage, count, range, shootSpeed, dimensions), entityToPlace(entityToPlace), sayCreator(sayCreator) { }

	PlacedOnLanding(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", string typeName = "NULL TYPE", RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false, float shootSpeed = 0.25f, Vec2f dimensions = vOne) :
		Item(baseClass, name, typeName, color, damage, count, range, shootSpeed, dimensions), entityToPlace(entityToPlace), sayCreator(sayCreator) { }

	Item Clone(int count) override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, sayCreator, shootSpeed, dimensions);
	}

	Item Clone() override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, sayCreator, shootSpeed, dimensions);
	}

	Item* Clone2(int count) override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, sayCreator, shootSpeed, dimensions);
	}

	Item* Clone2() override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, sayCreator, shootSpeed, dimensions);
	}

	void OnDeath(Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType) override
	{
		unique_ptr<Entity> placedEntity = entityToPlace->Clone(pos, up, creator);
		if (sayCreator)
			placedEntity->name += " from " + creatorName;
		game->entities->push_back(std::move(placedEntity));
	}
};

class ExplodeOnLanding : public Item
{
public:
	int explosionDamage;
	Vec2f explosionDimensions;

	ExplodeOnLanding(Vec2f explosionDimensions = vOne, int explosionDamage = 1, string name = "NULL", string typeName = "NULL TYPE", RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, float shootSpeed = 0.25f, Vec2f dimensions = vOne) :
		Item(name, typeName, color, damage, count, range, shootSpeed, dimensions), explosionDimensions(explosionDimensions), explosionDamage(explosionDamage) { }

	ExplodeOnLanding(Item* baseClass, Vec2f explosionDimensions = vOne, int explosionDamage = 1, string name = "NULL", string typeName = "NULL TYPE", RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, float shootSpeed = 0.25f, Vec2f dimensions = vOne) :
		Item(baseClass, name, typeName, color, damage, count, shootSpeed, range, dimensions), explosionDimensions(explosionDimensions), explosionDamage(explosionDamage) { }

	virtual Item Clone(int count)
	{
		return ExplodeOnLanding(baseClass, explosionDimensions, explosionDamage, name, typeName, color, damage, count, range, shootSpeed, dimensions);
	}

	virtual Item Clone()
	{
		return ExplodeOnLanding(baseClass, explosionDimensions, explosionDamage, name, typeName, color, damage, count, range, shootSpeed, dimensions);
	}

	virtual Item* Clone2(int count)
	{
		return new ExplodeOnLanding(baseClass, explosionDimensions, explosionDamage, name, typeName, color, damage, count, range, shootSpeed, dimensions);
	}

	virtual Item* Clone2()
	{
		return new ExplodeOnLanding(baseClass, explosionDimensions, explosionDamage, name, typeName, color, damage, count, range, shootSpeed, dimensions);
	}

	void OnDeath(Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType) override
	{
		game->entities->push_back(make_unique<ExplodeNextFrame>(explosionDamage, explosionDimensions, color, pos, name + string(" shot by ") + creatorName, creator));
		game->entities->push_back(make_unique<FadeOut>(0.5f, pos, explosionDimensions, color));
	}
};

class CorruptOnKill : public PlacedOnLanding
{
public:
	using PlacedOnLanding::PlacedOnLanding;

	void OnDeath(Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType) override
	{
		if (callType == 2)
			PlacedOnLanding::OnDeath(pos, creator, creatorName, callReason, callType);
		else
			Item::OnDeath(pos, creator, creatorName, callReason, callType);
	}
};

namespace Hazards
{
	FadeOutPuddle* leadPuddle = new FadeOutPuddle(3.0f, 1, 0.2f, vZero, vOne * 3, RGBA(80, 43, 92));
}

namespace Resources
{
	ExplodeOnLanding* ruby = new ExplodeOnLanding(vOne * 5, 4, "Ruby", "Ammo", RGBA(168, 50, 100), 4);
	ExplodeOnLanding* emerald = new ExplodeOnLanding(vOne * 15, 2, "Emerald", "Ammo", RGBA(65, 224, 150), 2);
	ExplodeOnLanding* topaz = new ExplodeOnLanding(vOne * 7, 3, "Topaz", "Ammo", RGBA(255, 200, 0), 3, 1, 15.0f, 0.25f, vOne * 3);
	ExplodeOnLanding* saphire = new ExplodeOnLanding(vOne * 3, 1, "Saphire", "Ammo", RGBA(78, 25, 212), 0, 1, 15.0f, 0.0625f);
	PlacedOnLanding* lead = new PlacedOnLanding(Hazards::leadPuddle, "Lead", "Deadly Ammo", RGBA(80, 43, 92), 0, 1, 15.0f, true);
}

namespace Collectibles
{
	Collectible* ruby = new Collectible(*Resources::ruby);
	Collectible* emerald = new Collectible(*Resources::emerald);
	Collectible* topaz = new Collectible(*Resources::topaz);
	Collectible* saphire = new Collectible(*Resources::saphire);
	Collectible* lead = new Collectible(*Resources::lead);
}