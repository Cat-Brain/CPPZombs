#include "LightSource.h"
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
	iVec2 pos;

	Chunk(iVec2 pos = vZero) :
		vector{}, pos(pos) { }

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
	vector<Chunk> chunks;
	vector<Chunk*> renderedChunks;

	Entities() :
		vector(0), addedEntity(false), index(0)
	{ }

	void push_back(unique_ptr<Entity> entity)
	{
		addedEntity = true;
		if (entity->isCollectible)
			collectibles.push_back(entity.get());
		else
		{
			index++;
			sortedNCEntities.insert(sortedNCEntities.begin(), entity.get());
		}
		int index = static_cast<int>(size());
		vector<int> chunkOverlaps = FindCreateChunkOverlaps(entity->pos, entity->radius);
		for (int chunk : chunkOverlaps)
			chunks[chunk].push_back(index);

		vector<unique_ptr<Entity>>::push_back(std::move(entity));
	}

#pragma region Overlaps and collisionstuff
	int ChunkAtPos(iVec2 pos)
	{
		for (int i = 0; i < chunks.size(); i++)
			if (chunks[i].pos == pos)
				return i;
		return -1;
	}

	vector<int> MainChunkOverlaps(iVec2 minPos, iVec2 maxPos) // In chunk coords, do NOT plug in normal space coords.
	{
		vector<int> result((maxPos.x - minPos.x + 1) * (maxPos.y - minPos.y + 1));
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				result[i++] = ChunkAtPos(iVec2(x * CHUNK_WIDTH, y * CHUNK_WIDTH));
		return result;
	}

	vector<int> FindCreateChunkOverlapsMain(iVec2 minPos, iVec2 maxPos) // A safer version of MainChunksOveraps.
	{
		vector<int> result((maxPos.x - minPos.x + 1) * (maxPos.y - minPos.y + 1));
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
			{
				int chunk = ChunkAtPos(iVec2(x * CHUNK_WIDTH, y * CHUNK_WIDTH));
				if (chunk != -1)
				{
					result[i++] = chunk;
					continue;
				}
				result[i++] = static_cast<int>(chunks.size());
				chunks.push_back(Chunk(iVec2(x * CHUNK_WIDTH, y * CHUNK_WIDTH)));
			}
		return result;
	}

	vector<int> ChunkOverlaps(Vec2 pos, float radius)
	{
		std::pair<iVec2, iVec2> minMaxPos = Chunk::MinMaxPos(pos, radius);
		return MainChunkOverlaps(minMaxPos.first, minMaxPos.second);
	}

	vector<int> FindCreateChunkOverlaps(Vec2 pos, float radius)
	{
		std::pair<iVec2, iVec2> minMaxPos = Chunk::MinMaxPos(pos, radius);
		return FindCreateChunkOverlapsMain(minMaxPos.first, minMaxPos.second);
	}

	/*Entity* FindNearestEnemy(iVec2 pos, iVec2 farthestDimensions)
	{
		vector<Entity*> nearbyEntities = FindCorpOverlaps(pos, farthestDimensions);
		float currentBestDist = 9999.0f; // Sqr magnitude not real magnitude.
		Entity* currentBest = nullptr;
		for (Entity* entity : nearbyEntities)
		{
			float dist;
			if (entity->isEnemy && (dist = glm::length2(Vec2(pos - entity->pos))) < currentBestDist)
			{
				currentBestDist = dist;
				currentBest = entity;
			}
		}
		return currentBest;
	}*/

	vector<Entity*> FindCorpOverlaps(vector<int> chunkOverlaps, Vec2 pos, float radius)
	{
		vector<Entity*> overlaps{};
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter]->corporeal && (*this)[*iter]->active && (*this)[*iter]->Overlaps(pos, radius) &&
					find(overlaps.begin(), overlaps.end(), (*this)[*iter].get()) == overlaps.end()) overlaps.push_back((*this)[*iter].get());
		return overlaps;
	}

	vector<Entity*> FindCorpOverlaps(Vec2 pos, float radius)
	{
		return FindCorpOverlaps(FindCreateChunkOverlaps(pos, radius), pos, radius);
	}

	vector<Entity*> FindIncorpOverlaps(vector<int> chunkOverlaps, Vec2 pos, float radius)
	{
		vector<Entity*> overlaps{};
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter]->corporeal && (*this)[*iter]->active && (*this)[*iter]->Overlaps(pos, radius)) overlaps.push_back((*this)[*iter].get());
		return overlaps;
	}

	vector<Entity*> FindIncorpOverlaps(Vec2 pos, float radius)
	{
		return FindIncorpOverlaps(FindCreateChunkOverlaps(pos, radius), pos, radius);
	}

	vector<Entity*> FindAllOverlaps(vector<int> chunkOverlaps, Vec2 pos, float radius)
	{
		vector<Entity*> overlaps{};
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter]->Overlaps(pos, radius) && (*this)[*iter]->active &&
					find(overlaps.begin(), overlaps.end(), (*this)[*iter].get()) == overlaps.end()) overlaps.push_back((*this)[*iter].get());
		return overlaps;
	}

	vector<Entity*> FindAllOverlaps(Vec2 pos, float radius)
	{
		return FindAllOverlaps(FindCreateChunkOverlaps(pos, radius), pos, radius);
	}

	std::pair<vector<Entity*>, vector<Entity*>> FindPairOverlaps(vector<int> chunkOverlaps, Vec2 pos, float radius) // Returns {corporeals, incorporeals}
	{
		vector<Entity*> corporeals{}, incorporeals{};
		for (int chunk : chunkOverlaps)
			for (vector<int>::iterator iter = chunks[chunk].begin(); iter != chunks[chunk].end(); iter++)
				if ((*this)[*iter]->Overlaps(pos, radius) && (*this)[*iter]->active &&
					(((*this)[*iter]->corporeal && find(corporeals.begin(), corporeals.end(), (*this)[*iter].get()) == corporeals.end()) ||
						(!(*this)[*iter]->corporeal && find(incorporeals.begin(), incorporeals.end(), (*this)[*iter].get()) == incorporeals.end())))
				{
					if ((*this)[*iter]->corporeal)
						corporeals.push_back((*this)[*iter].get());
					else
						incorporeals.push_back((*this)[*iter].get());
				}
		return { corporeals, incorporeals };
	}

	std::pair<vector<Entity*>, vector<Entity*>> FindPairOverlaps(Vec2 pos, float radius)
	{
		return FindPairOverlaps(FindCreateChunkOverlaps(pos, radius), pos, radius);
	}

	// Add more overlap functions.
#pragma endregion

	void SortEntities()
	{
		int length = static_cast<int>(size());
		for (unique_ptr<Entity>& entity : *this)
			length -= int(entity->isCollectible);
		vector<EntityIndex> unsortedToSorted = vector<EntityIndex>(length);
		for (int i = 0, j = 0; i < size(); i++)
			if (!(*this)[i]->isCollectible)
			{
				unsortedToSorted[j] = EntityIndex(i, (*this)[i]->sortLayer);
				j++;
			}
		std::sort(unsortedToSorted.begin(), unsortedToSorted.end());

		sortedNCEntities = vector<Entity*>(length);
		for (int i = 0, j = 0; i < size(); i++)
			if (!(*this)[i]->isCollectible)
			{
				sortedNCEntities[j] = (*this)[unsortedToSorted[j].index].get();
				j++;
			}

		collectibles = vector<Entity*>(size() - length);
		length = 0;
		for (unique_ptr<Entity>& entity : *this)
			if (entity->isCollectible)
				collectibles[length++] = entity.get();
		addedEntity = false;
	}

	void Update()
	{
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
		std::pair<vector<Entity*>, vector<Entity*>> toRenderPair = FindPairOverlaps(game->PlayerPos(), game->zoom * 2 + 1); // Collectibles then NCs.
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
			if (collectibles[index]->dActive && collectibles[index]->shouldUI)
			{
				collectibles[index]->UIUpdate();
			}

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->dActive && sortedNCEntities[index]->shouldUI)
			{
				sortedNCEntities[index]->UIUpdate();
			}
	}

	void SubScatUpdate()
	{
		std::pair<vector<Entity*>, vector<Entity*>> toRenderPair = FindPairOverlaps(game->PlayerPos(), game->zoom * 0.5f + 0.5f); // Collectibles then NCs.
		// Collectibles
		for (Entity* entity : toRenderPair.second)
			entity->SubScatUpdate();
		// Normal entities
		for (Entity* entity : toRenderPair.first)
			entity->SubScatUpdate();
	}

	void Remove(Entity* entityToRemove)
	{
		// Remove from sortedNCEntities or from collectibles.
		if (!entityToRemove->isCollectible)
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
		vector<int> chunkOverlaps = ChunkOverlaps(entityToRemove->pos, entityToRemove->radius);
		for (int chunk : chunkOverlaps)
			chunks[chunk].erase(find(chunks[chunk].begin(), chunks[chunk].end(), removalIndex));

		for (Chunk& chunk : chunks)
			for (int i = 0; i < chunk.size(); i++)
				chunk[i] -= int(chunk[i] > removalIndex);

		// Remove from main list from which the rest derive.
		erase(mainPos);
		
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

	void Vacuum(Vec2 pos, float vacDist, float speed)
	{
		for (Entity* collectible : collectibles)
		{
			float distance = Distance(pos, collectible->pos);
			if (collectible->active && distance > 0 && distance <= vacDist + collectible->radius)
				collectible->SetPos(collectible->pos + Normalized(Vec2(pos - collectible->pos)) * game->dTime * speed / collectible->mass);
		}
	}

	void VacuumNC(Vec2 pos, float vacDist, float speed)
	{
		for (Entity* entity : sortedNCEntities)
		{
			float distance = Distance(pos, entity->pos);
			if (entity->active && entity->corporeal && distance > 0 && distance <= vacDist + entity->radius)
				entity->SetPos(entity->pos + Normalized(Vec2(pos - entity->pos)) * game->dTime * speed / entity->mass);
		}
	}

	void VacuumBoth(Vec2 pos, float vacDist, float speed)
	{
		for (unique_ptr<Entity>& entity : *this)
		{
			float distance = Distance(pos, entity->pos);
			if (entity->active && (entity->corporeal || entity->isCollectible) && distance > 0 && distance <= vacDist + entity->radius)
				entity->SetPos(entity->pos + Normalized(Vec2(pos - entity->pos)) * game->dTime * speed / entity->mass);
		}
	}

	void VacuumCone(Vec2 pos, Vec2 dir, int vacDist, float fov)
	{
		for (Entity* collectible : collectibles)
		{
			int distance = Squistance(pos, collectible->pos);
			if (collectible->active && distance > 0 && distance <= vacDist && glm::dot(dir, Normalized(Vec2(Vec2(collectible->pos - pos) + dir))) >= 1 - fov)
				collectible->SetPos(collectible->pos + Normalized(Vec2(pos - collectible->pos)) * game->dTime);
		}
	}
};

#pragma region Post Entities functions

int Entity::DealDamage(int damage, Entity* damageDealer)
{
	if (damage > 0)
		game->entities->particles.push_back(make_unique<SpinText>(Vec2(pos) + Vec2(RandFloat(), RandFloat()) * Vec2(radius * 2 - 1) - Vec2(up), static_cast<float>(damage), to_string(damage),
			RGBA(damageDealer->color.r, damageDealer->color.g, damageDealer->color.b, damageDealer->color.a / 2),
			static_cast<float>(COMMON_TEXT_SCALE), RandFloat() * 5.0f, COMMON_TEXT_SCALE * (RandFloat() * 0.25f + 0.25f)));
	
	health -= damage;
	if (health <= 0)
	{
		DestroySelf(damageDealer);
		return 1;
	}
	return 0;
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

bool Entity::TryMove(Vec2 direction, float force, Entity* ignore, Entity** hitEntity) // returns index of hit item.
{
	Vec2 newPos = pos + direction;
	/*vector<int> chunkOverlaps;
	if (!corporeal)
	{
		SetPos(newPos);
		return true;
	}
	if (force >= mass && direction != Vec2(0))
	{
		chunkOverlaps = game->entities->FindCreateChunkOverlaps(newPos, radius);
		vector<Entity*> overlaps = game->entities->FindCorpOverlaps(chunkOverlaps, newPos, radius);
		for (Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != entity->creator || creator == nullptr))
			{
				if (hitEntity != nullptr)
					*hitEntity = entity;
				if (!entity->TryMove(direction, force - mass, ignore) && !entity->Overlaps(pos, radius))
				{
					// something in front of them, however if they're stuck, we want to let them move anyways.
					/*vector<Entity*> overlaps2 = game->entities->FindCorpOverlaps(pos, radius);
					bool successful = false;
					for (Entity* entity2 : overlaps2)
						if (entity2 != ignore && entity2 != this && (creator != entity2->creator || creator == nullptr) &&
							force - mass > entity2->mass)
						{
							successful = true;
							break;
						}
					if (successful)
						break;/
					return false; // The entity is not stuck inside another entity and are blocked.
				}
			}
	}
	else return false;*/

	SetPos(newPos);

	return true;
}

void Entity::SetPos(Vec2 newPos)
{
	std::pair<iVec2, iVec2> minMaxOldPos = Chunk::MinMaxPos(pos, radius);
	std::pair<iVec2, iVec2> minMaxNewPos = Chunk::MinMaxPos(newPos, radius);
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
	vector<Entity*> entities = game->entities->FindCorpOverlaps(pos, radius);
	for (Entity* entity : entities)
		if (entity->pos != pos && entity->creator != this && creator != entity)
			OverlapRes::CircleOR(this, entity);
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

	ExplodeNextFrame(int damage = 1, float explosionRadius = 0.5f, RGBA color = RGBA(), iVec2 pos = vZero, string name = "NULL NAME", Entity* creator = nullptr) :
		Entity(pos, 0.5f, color, color, 1, 1, 1, string("Explosion from ") + name), damage(damage), explosionRadius(explosionRadius), startTime(tTime)
	{
		update = UPDATE::EXPLODENEXTFRAMEU;
		this->creator = creator;
		corporeal = false;
	}
};

class FadeOutPuddle : public Entity
{
public:
	int damage;
	float startTime, totalFadeTime, timePer, lastTime;

	FadeOutPuddle(float totalFadeTime = 1.0f, int damage = 1, float timePer = 1.0f, Vec2 pos = Vec2(0, 0),
		float radius = 0.5f, RGBA color = RGBA()) :
		Entity(pos, radius, color, color, 1, 1, 1, "Puddle"),
		totalFadeTime(totalFadeTime), damage(damage), startTime(tTime), timePer(timePer), lastTime(tTime)
	{
		update = UPDATE::FADEOUTPUDDLEU;
		dUpdate = DUPDATE::FADEOUTPUDDLEDU;
		corporeal = false;
	}

	FadeOutPuddle(FadeOutPuddle* baseClass, Vec2 pos) :
		FadeOutPuddle(*baseClass) {
		this->pos = pos;
		startTime = tTime;
	}

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<FadeOutPuddle>(this, pos);
	}
};

class FadeOutGlow : public FadeOut
{
public:
	float startRange;
	LightSource* lightSource;

	FadeOutGlow(float range, float totalFadeTime = 1.0f, iVec2 pos = vZero, float radius = 0.5f, RGBA color = RGBA()) :
		FadeOut(totalFadeTime, pos, radius, color), startRange(range)
	{
		dUpdate = DUPDATE::FADEOUTGLOWDU;
		onDeath = ONDEATH::FADEOUTGLOWOD;
		game->entities->lightSources.push_back(make_unique<LightSource>(pos, JRGB(color.r, color.g, color.b), range));
		lightSource = game->entities->lightSources[game->entities->lightSources.size() - 1].get();
	}
};

class VacuumeFor : public Entity
{
public:
	float startTime, timeTill, vacDist, vacSpeed;

	VacuumeFor(Vec2 pos, float timeTill, float vacDist, float vacSpeed, RGBA color) :
		Entity(pos, vacDist, color), startTime(tTime), timeTill(timeTill), vacDist(vacDist), vacSpeed(vacSpeed)
	{
		update = UPDATE::VACUUMEFORU;
		corporeal = false;
	}

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<VacuumeFor>(pos, timeTill, vacDist, vacSpeed, color);
	}
};

namespace Updates
{
	void ExplodeNextFrameU(Entity* entity)
	{
		ExplodeNextFrame* explosion = static_cast<ExplodeNextFrame*>(entity);
		if (tTime != explosion->startTime)
		{
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(explosion->pos, explosion->explosionRadius);
			for (Entity* entity : hitEntities)
				if (entity != explosion && entity != explosion->creator)
					entity->DealDamage(explosion->damage, explosion);
			for (int i = 0; i < EXPLOSION_PARTICLE_COUNT; i++)
			{
				float rotation = RandFloat() * PI_F * 2;
				game->entities->particles.push_back(make_unique<VelocityCircle>(0.25f, explosion->pos, Vec2(sinf(rotation), cosf(rotation)) * EXPLOSION_PARTICLE_SPEED,
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
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(puddle->pos, puddle->radius);
			for (Entity* entity : hitEntities)
				entity->DealDamage(puddle->damage, puddle);
		}
		if (tTime - puddle->startTime > puddle->totalFadeTime)
			puddle->DestroySelf(puddle);
	}

	void VacuumeForU(Entity* entity)
	{
		VacuumeFor* vac = static_cast<VacuumeFor*>(entity);

		if (tTime - vac->startTime > vac->timeTill)
		{
			vac->DestroySelf(nullptr);
			return;
		}

		game->entities->VacuumBoth(vac->pos, vac->vacDist, vac->vacSpeed);
	}
}

namespace DUpdates
{
	void FadeOutPuddleDU(Entity* entity)
	{
		FadeOutPuddle* puddle = static_cast<FadeOutPuddle*>(entity);
		puddle->color.a = 255 - static_cast<uint8_t>((tTime - puddle->startTime) * 255 / puddle->totalFadeTime);
		puddle->DUpdate(DUPDATE::ENTITYDU);
	}

	void FadeOutGlowDU(Entity* entity)
	{
		FadeOutGlow* glow = static_cast<FadeOutGlow*>(entity);
		glow->lightSource->range = glow->startRange * glow->Opacity();
		glow->DUpdate(DUPDATE::FADEOUTDU);
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
		float shootSpeed = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(entityToPlace->name, typeName, intType, entityToPlace->color, entityToPlace->subScat, damage, count, range, shootSpeed, radius,
			corporeal, shouldCollide, mass, health), entityToPlace(entityToPlace), sayCreator(sayCreator)
	{
		itemOD = ITEMOD::PLACEDONLANDING;
	}

	PlacedOnLanding(Entity* entityToPlace, string name, string typeName, int intType = 0, RGBA color = RGBA(), RGBA subScat = RGBA(),
		int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false, float shootSpeed = 0.25f, float radius = 0.5f,
		bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(name, typeName, intType, color, subScat, damage, count, range, shootSpeed, radius, corporeal, shouldCollide, mass, health),
		entityToPlace(entityToPlace), sayCreator(sayCreator)
	{
		itemOD = ITEMOD::PLACEDONLANDING;
	}

	PlacedOnLanding(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", string typeName = "NULL TYPE", int intType = 0,
		RGBA color = RGBA(), RGBA subScat = RGBA(), int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false,
		float shootSpeed = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(baseClass, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, radius, corporeal, shouldCollide, mass, health),
		entityToPlace(entityToPlace), sayCreator(sayCreator)
	{
		itemOD = ITEMOD::PLACEDONLANDING;
	}

	Item Clone(int count) override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, intType, color, subScat, damage, count, range, sayCreator, shootSpeed, radius, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2(int count) override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, intType, color, subScat, damage, count, range, sayCreator, shootSpeed, radius, corporeal, shouldCollide, mass, health);
	}
};

class CorruptOnKill : public PlacedOnLanding
{
public:
	CorruptOnKill(Entity* entityToPlace, string typeName, int intType = 0, int damage = 0, int count = 1, float range = 15.0f, bool sayCreator = false,
		float shootSpeed = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		PlacedOnLanding(entityToPlace, typeName, intType, damage, count, range, sayCreator,
			shootSpeed, radius, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::CORRUPTONKILL;
	}

	CorruptOnKill(Entity* entityToPlace, string name, string typeName, int intType = 0, RGBA color = RGBA(), RGBA subScat = RGBA(),
		int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false, float shootSpeed = 0.25f, float radius = 0.5f,
		bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		PlacedOnLanding(entityToPlace, name, typeName, intType, color, subScat, damage, count, range, sayCreator, shootSpeed, radius, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::CORRUPTONKILL;
	}

	CorruptOnKill(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", string typeName = "NULL TYPE", int intType = 0,
		RGBA color = RGBA(), RGBA subScat = RGBA(), int damage = 1, int count = 1, float range = 15.0f, bool sayCreator = false,
		float shootSpeed = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		PlacedOnLanding(baseClass, entityToPlace, name, typeName, intType, color, subScat, damage, count, range, sayCreator, shootSpeed, radius, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::CORRUPTONKILL;
	}
};

inline void CreateExplosion(iVec2 pos, float explosionRadius, RGBA color, string name, int damage, int explosionDamage, Entity* creator)
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
		RGBA color = RGBA(), RGBA subScat = RGBA(), int damage = 1, int count = 1, float range = 15.0f, float shootSpeed = 0.25f, float radius = 0.5f,
		bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(name, typeName, intType, color, subScat, damage, count, range, shootSpeed, radius, corporeal, shouldCollide, mass, health),
		explosionRadius(explosionRadius), explosionDamage(explosionDamage)
	{
		itemOD = ITEMOD::EXPLODEONLANDING;
	}

	ExplodeOnLanding(Item* baseClass, float explosionRadius = 0.5f, int explosionDamage = 1, string name = "NULL",
		string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), RGBA subScat = RGBA(), int damage = 1, int count = 1, float range = 15.0f,
		float shootSpeed = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(baseClass, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, radius, corporeal, shouldCollide, mass, health),
		explosionRadius(explosionRadius), explosionDamage(explosionDamage) { }

	Item Clone(int count) override
	{
		return ExplodeOnLanding(baseClass, explosionRadius, explosionDamage, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, radius, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2(int count) override
	{
		return new ExplodeOnLanding(baseClass, explosionRadius, explosionDamage, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, radius);
	}
};

namespace ItemODs
{
	void ItemOD(Item* item, Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		game->entities->push_back(make_unique<Collectible>(*item, pos));
	}

	void PlacedOnLandingOD(Item* item, Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		PlacedOnLanding* pOL = static_cast<PlacedOnLanding*>(item);

		if (((Entity*)game->player)->Overlaps(pos, pOL->radius))
		{
			pOL->OnDeath(ITEMOD::ITEM, pos, creator, creatorName, callReason, callType);
			return;
		}
		unique_ptr<Entity> placedEntity = pOL->entityToPlace->Clone(pos, up, creator);
		if (pOL->sayCreator)
			placedEntity->name += " from " + creatorName;
		game->entities->push_back(std::move(placedEntity));
	}

	void CorruptOnKillOD(Item* item, Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		item->OnDeath(callType == 2 ? ITEMOD::PLACEDONLANDING : ITEMOD::ITEM, pos, creator, creatorName, callReason, callType);
	}

	void ExplodeOnLandingOD(Item* item, Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		ExplodeOnLanding* explosion = static_cast<ExplodeOnLanding*>(item);
		CreateExplosion(pos, explosion->explosionRadius, explosion->color, explosion->name + string(" shot by " + creatorName),
			explosion->damage, explosion->explosionDamage, creator);
	}
}

namespace Hazards
{
	FadeOutPuddle* leadPuddle = new FadeOutPuddle(3.0f, 1, 0.2f, vZero, 1.5f, RGBA(80, 43, 92));
	VacuumeFor* vacuumPuddle = new VacuumeFor(vZero, 2, 10, 4, RGBA(255, 255, 255, 51));
}

namespace Resources
{
	ExplodeOnLanding* ruby = new ExplodeOnLanding(2.5f, 4, "Ruby", "Ammo", 1, RGBA(168, 50, 100), RGBA(0, 10, 10), 4);
	ExplodeOnLanding* emerald = new ExplodeOnLanding(7.5f, 2, "Emerald", "Ammo", 1, RGBA(65, 224, 150), RGBA(10, 0, 10), 2);
	ExplodeOnLanding* topaz = new ExplodeOnLanding(3.5f, 3, "Topaz", "Ammo", 1, RGBA(255, 200, 0), RGBA(0, 0, 10), 3, 1, 15.0f, 0.25f, 1.5f);
	ExplodeOnLanding* sapphire = new ExplodeOnLanding(1.5f, 1, "Sapphire", "Ammo", 1, RGBA(78, 25, 212), RGBA(10, 10, 0), 0, 1, 15.0f, 0.0625f);
	PlacedOnLanding* lead = new PlacedOnLanding(Hazards::leadPuddle, "Lead", "Deadly Ammo", 1, RGBA(80, 43, 92), RGBA(0, 10, 5), 0, 1, 15.0f, true);
	PlacedOnLanding* vacuumium = new PlacedOnLanding(Hazards::vacuumPuddle, "Vacuumium", "Push Ammo", 1, RGBA(255, 255, 255), RGBA(5, 5, 5), 0);
}

namespace Collectibles
{
	Collectible* ruby = new Collectible(*Resources::ruby);
	Collectible* emerald = new Collectible(*Resources::emerald);
	Collectible* topaz = new Collectible(*Resources::topaz);
	Collectible* sapphire = new Collectible(*Resources::sapphire);
	Collectible* lead = new Collectible(*Resources::lead);
	Collectible* vacuumium = new Collectible(*Resources::vacuumium);
}