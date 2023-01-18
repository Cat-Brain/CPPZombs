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

#define CHUNK_WIDTH 16
#define MAP_WIDTH 100 // Defined in chunks.
#define MAP_WIDTH_TRUE CHUNK_WIDTH * MAP_WIDTH
class Chunk : public vector<Entity*>
{
public:
	Vec2 pos;

	Chunk(vector<Entity*> entities = {}, Vec2 pos = vZero) :
		vector(entities), pos(pos) { }

	bool Overlaps(Vec2 pos, Vec2 dimensions)
	{
		return pos.x - dimensions.x >= this->pos.x + dimensions.x && pos.x < this->pos.x + CHUNK_WIDTH &&
			pos.y - dimensions.y >= this->pos.y + dimensions.y && pos.y < this->pos.y + CHUNK_WIDTH;
	}
};

class Entities : public vector<Entity*>
{
protected:
	bool updatingProjectiles;
	int index;
	int currentUpdatingType;

public:
	Entities() :
		vector(0)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
			for (int y = 0; y < MAP_WIDTH; y++)
				chunks[x][y].pos = Vec2(x * CHUNK_WIDTH, y * CHUNK_WIDTH);
	}

	bool addedEntity;
	vector<Entity*> sortedNCEntities; // The NC stands for Non-Collectible.
	vector<Entity*> collectibles; // sortedNCEntities and collectibles are the most accurate, the others are less so.
	vector<Entity*> projectiles, nonProjectiles;
	vector<Entity*> enemies, nonEnemies;
	vector<Entity*> corporeals, incorporeals;
	Chunk chunks[MAP_WIDTH][MAP_WIDTH];

	vector<Chunk*> ChunkOverlaps(Vec2 pos, Vec2 dimensions)
	{
		Vec2 minPos = Vec2(max((pos.x - dimensions.x) / CHUNK_WIDTH, 0), max((pos.y - dimensions.y) / CHUNK_WIDTH, 0)),
			maxPos = Vec2(min((pos.x + dimensions.x) / CHUNK_WIDTH, MAP_WIDTH), min((pos.y + dimensions.y) / CHUNK_WIDTH, MAP_WIDTH));
		vector<Chunk*> result((maxPos.x - minPos.x + 1) * (maxPos.y - minPos.y + 1));
		for (int i = 0, x = minPos.x; x <= maxPos.x; x++)
			for (int y = minPos.y; y <= maxPos.y; y++)
				result[i++] = &chunks[x][y];
		return result;
	}

	void push_back(Entity* entity)
	{
		entity->pos.x = Clamp(entity->pos.x, 0, MAP_WIDTH_TRUE);
		entity->pos.y = Clamp(entity->pos.y, 0, MAP_WIDTH_TRUE);

		addedEntity = true;
		vector<Entity*>::push_back(entity);
		if (entity->IsCollectible())
			collectibles.push_back(entity);
		else
		{
			index++;
			sortedNCEntities.insert(sortedNCEntities.begin(), entity);
			if (entity->Corporeal())
				corporeals.push_back(entity);
		}

		vector<Chunk*> chunkOverlaps = ChunkOverlaps(entity->pos, entity->dimensions);
		for (Chunk* chunk : chunkOverlaps)
			chunk->push_back(entity);
	}

	Entity* FindNearestEnemy(Vec2 pos)
	{
		float currentBestDist = 9999.0f;
		Entity* currentBest = nullptr;
		for (Entity* entity : *this)
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

	vector<Entity*>::iterator FindCorpPos(Vec2 pos)
	{
		for (vector<Entity*>::iterator iter = corporeals.begin(); iter != corporeals.end(); iter++)
			if ((*iter)->pos == pos)
				return iter;
		return corporeals.end();
	}

	vector<Entity*> FindCorpOverlaps(vector<Chunk*> chunkOverlaps, Vec2 pos, Vec2 hDim)
	{
		vector<Entity*> overlaps{};
		for (Chunk* chunk : chunkOverlaps)
			for (vector<Entity*>::iterator iter = chunk->begin(); iter != chunk->end(); iter++)
				if ((*iter)->Corporeal() && (*iter)->Overlaps(pos, hDim)) overlaps.push_back(*iter);
		return overlaps;
	}

	vector<Entity*> FindCorpOverlaps(Vec2 pos, Vec2 hDim)
	{
		return FindCorpOverlaps(ChunkOverlaps(pos, hDim), pos, hDim);
	}

	vector<Entity*>::iterator FindIncorpPos(Vec2 pos)
	{
		for (vector<Entity*>::iterator iter = incorporeals.begin(); iter != incorporeals.end(); iter++)
			if ((*iter)->pos == pos)
				return iter;
		return incorporeals.end();
	}

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

	void Update(Game* game, float dTime)
	{
		if (addedEntity)
			SortEntities();

		int counterOne, counterTwo; // Will be used many times, so lets just create 'em at the start.
#pragma region Enemies and Non-Enemies
		counterOne = 0; // Enemy count
		for (Entity* entity : *this)
			counterOne += int(entity->IsEnemy());


		enemies = vector<Entity*>(counterOne); // Only one malloc per sorted list per frame =]
		nonEnemies = vector<Entity*>(size() - counterOne);

		counterOne = 0; // furthest empty index of enemies.
		counterTwo = 0; // furthest empty index of nonEnemies.

		for (Entity* entity : *this)
		{
			if (entity->IsEnemy())
			{
				enemies[counterOne] = entity;
				counterOne++;
			}
			else
			{
				nonEnemies[counterTwo] = entity;
				counterTwo++;
			}
		}
#pragma endregion

#pragma region Projectiles and Non-Projectiles
		counterOne = 0; // Projectile count
		for (Entity* entity : *this)
			counterOne += int(entity->IsProjectile());


		projectiles = vector<Entity*>(counterOne); // Only one malloc per sorted list per frame =]
		nonProjectiles = vector<Entity*>(size() - counterOne);

		counterOne = 0; // furthest empty index of projectiles.
		counterTwo = 0; // furthest empty index of nonProjectiles.

		for (Entity* entity : *this)
		{
			if (entity->IsProjectile())
			{
				projectiles[counterOne] = entity;
				counterOne++;
			}
			else
			{
				nonProjectiles[counterTwo] = entity;
				counterTwo++;
			}
		}
#pragma endregion

#pragma region Corporeals and Incorporeals
		counterOne = 0; // Corporeal count
		for (Entity* entity : *this)
			counterOne += int(entity->Corporeal());


		corporeals = vector<Entity*>(counterOne); // Only one malloc per sorted list per frame =]
		incorporeals = vector<Entity*>(size() - counterOne);

		counterOne = 0; // furthest empty index of projectiles.
		counterTwo = 0; // furthest empty index of nonEnemies.

		for (Entity* entity : *this)
		{
			if (entity->Corporeal())
			{
				corporeals[counterOne] = entity;
				counterOne++;
			}
			else
			{
				incorporeals[counterTwo] = entity;
				counterTwo++;
			}
		}
#pragma endregion


		for (int i = 0; i < collectibles.size(); i++)
			if (collectibles[i]->active)
				collectibles[i]->Update(game, dTime);

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->active)
				sortedNCEntities[index]->Update(game, dTime);

		if (addedEntity)
			SortEntities();
	}

	void DUpdate(Game* game, float dTime)
	{
		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->dActive)
				collectibles[index]->DUpdate(game, dTime);

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->dActive)
				sortedNCEntities[index]->DUpdate(game, dTime);
	}

	void UIUpdate(Game* game, float dTime)
	{
		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->dActive && collectibles[index]->shouldUI)
				collectibles[index]->UIUpdate(game, dTime);

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->dActive && sortedNCEntities[index]->shouldUI)
				sortedNCEntities[index]->UIUpdate(game, dTime);
	}

	void Remove(Entity* entityToRemove)
	{
		erase(find(begin(), end(), entityToRemove));
		if (!entityToRemove->IsCollectible())
		{
			vector<Entity*>::iterator pos = find(sortedNCEntities.begin(), sortedNCEntities.end(), entityToRemove);
			index -= int(index >= distance(sortedNCEntities.begin(), pos)); // If index is past or at the position being removed then don't advance.
			sortedNCEntities.erase(pos);
			if (entityToRemove->Corporeal())
				corporeals.erase(find(corporeals.begin(), corporeals.end(), entityToRemove));
		}
		else
		{
			vector<Entity*>::iterator pos = find(collectibles.begin(), collectibles.end(), entityToRemove);
			collectibles.erase(pos);
		}
		vector<Chunk*> chunks = ChunkOverlaps(entityToRemove->pos, entityToRemove->dimensions);
		for (Chunk* chunk : chunks)
			chunk->erase(find(chunk->begin(), chunk->end(), entityToRemove));
	}

	void Vacuum(Vec2 pos, int vacDist)
	{
		for (Entity* collectible : collectibles)
		{
			int distance = Diagnistance(pos, collectible->pos);
			if (collectible->active && distance > 0 && distance <= vacDist)
			{
				collectible->pos += Squarmalized(pos - collectible->pos);
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
				collectible->pos += Squarmalized(pos - collectible->pos);
			}
		}
	}
};

#pragma region Post Entities functions

void Item::OnDeath(Entities* entities, Vec2 pos, Entity* creator, Entity* callReason, int callType)
{
	entities->push_back(new Collectible(*this, pos));
}

void Entity::DestroySelf(Game* game, Entity* damageDealer)
{
	if (shouldUI)
		game->MenuedEntityDied(this);
	game->entities->Remove(this);
	OnDeath(game->entities, damageDealer);
	if (holder != nullptr)
		holder->heldEntity = nullptr;
	delete this;
}

bool Entity::TryMove(Vec2 direction, int force, Entities* entities, Entity** hitEntity, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;
	vector<Chunk*> chunkOverlaps;
	if (newPos.x >= 0 && newPos.x < MAP_WIDTH_TRUE && newPos.y >= 0 && newPos.y < MAP_WIDTH_TRUE && force >= mass && direction != Vec2(0, 0))
	{
		chunkOverlaps = entities->ChunkOverlaps(newPos, dimensions);
		vector<Entity*> overlaps = entities->FindCorpOverlaps(chunkOverlaps, newPos, dimensions);
		for (Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != entity->creator || creator == nullptr))
			{
				if (hitEntity != nullptr)
					*hitEntity = entity;
				if (!entity->TryMove(direction, force - mass, entities, ignore) && !entity->Overlaps(pos, dimensions))
				{
					// something in front of them, however if they're stuck, we want to let them move anyways.
					vector<Entity*> overlaps2 = entities->FindCorpOverlaps(pos, dimensions);
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

	vector<Chunk*> currentChunkOverlaps = entities->ChunkOverlaps(pos, dimensions);
	for (Chunk* chunk : currentChunkOverlaps)
		chunk->erase(find(chunk->begin(), chunk->end(), this));
	pos = newPos;
	for (Chunk* chunk : chunkOverlaps)
		chunk->push_back(this);
	return true;
}

bool Entity::TryMove(Vec2 direction, int force, Entities* entities, Entity* ignore) // returns index of hit item.
{
	return TryMove(direction, force, entities, nullptr, ignore);
}

#pragma endregion


// Post entities definition entities:

class ExplodeNextFrame : public Entity
{
public:
	int damage;
	Vec2 explosionDimensions;
	float startTime;

	ExplodeNextFrame(int damage = 1, Vec2 explosionDimensions = vOne, Vec2 pos = vZero, string name = "NULL NAME", Entity* creator = nullptr) :
		Entity(pos, vOne, olc::WHITE, 1, 1, 1, string("Explosion from ") + name), damage(damage), explosionDimensions(explosionDimensions), startTime(tTime)
	{
		this->creator = creator;
	}


	void Update(Game* game, float dTime) override
	{
		if (tTime != startTime)
		{
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(pos, explosionDimensions);
			for (Entity* entity : hitEntities)
				if (entity != this && entity != creator)
					entity->DealDamage(damage, game, this);
			DestroySelf(game, this);
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
		Entity(pos, dimensions, color, 1, 1, 1, "Puddle from "),
		totalFadeTime(totalFadeTime), damage(damage), startTime(tTime), timePer(timePer), lastTime(tTime) { }

	FadeOutPuddle(FadeOutPuddle* baseClass, Vec2 pos) :
		FadeOutPuddle(*baseClass) {
		this->pos = pos;
		startTime = tTime;
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		FadeOutPuddle* newPuddle = new FadeOutPuddle(this, pos);
		if (creator != nullptr)
			newPuddle->name += creator->name;
		return newPuddle;
	}


	void Update(Game* game, float dTime) override
	{
		if (tTime - lastTime > timePer)
		{
			lastTime = tTime;
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(pos, dimensions);
			for (Entity* entity : hitEntities)
				entity->DealDamage(damage, game, this);
		}
		if (tTime - startTime > totalFadeTime)
			DestroySelf(game, nullptr);
	}
	void DUpdate(Game* game, float dTime) override
	{
		color.a = 255 - static_cast<uint8_t>((tTime - startTime) * 255 / totalFadeTime);
		Entity::DUpdate(game, dTime);
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
	Entity* entityToPlace;

	PlacedOnLanding(Entity* entityToPlace, string typeName, int damage = 0, int count = 1, float range = 15.0f, Vec2 dimensions = vOne) :
		Item(entityToPlace->name, typeName, entityToPlace->color, damage, count, range, dimensions), entityToPlace(entityToPlace) { }

	PlacedOnLanding(Entity* entityToPlace, string name, string typeName, Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f, Vec2 dimensions = vOne) :
		Item(name, typeName, color, damage, count, range, dimensions), entityToPlace(entityToPlace) { }

	PlacedOnLanding(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", string typeName = "NULL TYPE", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f, Vec2 dimensions = vOne) :
		Item(baseClass, name, typeName, color, damage, count, range, dimensions), entityToPlace(entityToPlace) { }

	Item Clone(int count) override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, dimensions);
	}

	Item Clone() override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, dimensions);
	}

	Item* Clone2(int count) override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, dimensions);
	}

	Item* Clone2() override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, typeName, color, damage, count, range, dimensions);
	}

	void OnDeath(Entities* entities, Vec2 pos, Entity* creator, Entity* callReason, int callType) override
	{
		entities->push_back(entityToPlace->Clone(pos, up, creator));
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

	void OnDeath(Entities* entities, Vec2 pos, Entity* creator, Entity* callReason, int callType) override
	{
		entities->push_back(new ExplodeNextFrame(damage, explosionDimensions, pos, name + string(" shot by ") + creator->name, creator));
		entities->push_back(new FadeOut(0.5f, pos, explosionDimensions, color));
	}
};

class CorruptOnKill : public PlacedOnLanding
{
public:
	using PlacedOnLanding::PlacedOnLanding;

	void OnDeath(Entities* entities, Vec2 pos, Entity* creator, Entity* callReason, int callType) override
	{
		if (callType == 2)
			PlacedOnLanding::OnDeath(entities, pos, creator, callReason, callType);
		else
			Item::OnDeath(entities, pos, creator, callReason, callType);
	}
};

namespace Hazards
{
	FadeOutPuddle* leadPuddle = new FadeOutPuddle(3.0f, 1, 0.2f, vZero, vOne * 3, Color(80, 43, 92));
}

namespace Resources
{
	ExplodeOnLanding* ruby = new ExplodeOnLanding(vOne * 3, "Ruby", "Ammo", Color(168, 50, 100), 3);
	ExplodeOnLanding* emerald = new ExplodeOnLanding(vOne * 5, "Emerald", "Ammo", Color(65, 224, 150), 2);
	ExplodeOnLanding* topaz = new ExplodeOnLanding(vOne * 4, "Topaz", "Ammo", Color(255, 200, 0), 1, 1, 15.0f, vOne * 2);
	PlacedOnLanding* lead = new PlacedOnLanding(Hazards::leadPuddle, "Lead", "Ammo", Color(80, 43, 92), 0);
}

namespace Collectibles
{
	Collectible* ruby = new Collectible(*Resources::ruby, vZero);
	Collectible* emerald = new Collectible(*Resources::emerald, vZero);
	Collectible* topaz = new Collectible(*Resources::topaz, vZero);
	Collectible* lead = new Collectible(*Resources::lead, vZero);
}