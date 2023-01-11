#include "Entity.h"

class Collectible : public Entity
{
public:
	Item baseItem;

	Collectible(Item baseItem, Vec2 pos = vZero) :
		Entity(pos, vOne, baseItem.color, 1, 1, 1, baseItem.name), baseItem(baseItem) { }

	Collectible(Item baseItem, Vec2 pos, Color color) :
		Entity(pos, vOne, color, 1, 1, 1, baseItem.name), baseItem(baseItem) { }

	Collectible(Collectible* baseClass, Vec2 pos) : Collectible(*baseClass) { this->pos = pos; }

	void DUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		game->Draw(ToRSpace(pos), color);
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return new Collectible(this, pos);
	}


	virtual Collectible* Clone(int count)
	{
		return new Collectible(baseItem.Clone(count), pos, color);
	}

	bool Corporeal() override
	{
		return false;
	}

	bool IsCollectible() override
	{
		return true;
	}
};

namespace Collectibles
{
	Collectible* copper = new Collectible(*Resources::copper, vZero);
	Collectible* iron = new Collectible(*Resources::iron, vZero);
}


#pragma region Other Collectible funcitons
vector<Entity*> EntitiesAtPos(Vec2 pos, vector<Entity*> entities)
{
	vector<Entity*> foundCollectibles(0);
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->pos == pos)
			foundCollectibles.push_back(*i);
	return foundCollectibles;
}

Entity* FindAEntity(Vec2 pos, vector<Entity*> entities)
{
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->pos == pos)
			return *i;
	return nullptr;
}
#pragma endregion



struct EntityIndex // For sorting.
{
	int index, valueForSorting;

	EntityIndex(int index = 0, int valueForSorting = 0) : index(index), valueForSorting(valueForSorting) {}

	bool operator < (const EntityIndex& other) const
	{
		return (valueForSorting < other.valueForSorting);
	}
};

class Entities : public vector<Entity*>
{
protected:
	bool updatingProjectiles;
	int index;
	int currentUpdatingType;

public:
	using vector<Entity*>::vector;

	bool addedEntity;
	vector<Entity*> sortedNCEntities; // The NC stands for Non-Collectible.
	vector<Entity*> collectibles; // sortedNCEntities and collectibles are the most accurate, the others are less so.
	vector<Entity*> projectiles, nonProjectiles;
	vector<Entity*> enemies, nonEnemies;
	vector<Entity*> corporeals, incorporeals;

	void push_back(Entity* entity)
	{
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

	vector<Entity*> FindCorpOverlaps(Vec2 pos, Vec2 hDim)
	{
		vector<Entity*> overlaps(0);
		for (vector<Entity*>::iterator iter = corporeals.begin(); iter != corporeals.end(); iter++)
			if ((*iter)->Overlaps(pos, hDim)) overlaps.push_back(*iter);
		return overlaps;
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

	void Update(Game* game, int frameCount, Inputs inputs, float dTime)
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
				collectibles[i]->Update(game, this, frameCount, inputs, dTime);

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->active)
				sortedNCEntities[index]->Update(game, this, frameCount, inputs, dTime);

		if (addedEntity)
			SortEntities();
	}

	void DUpdate(Game* game, int frameCount, Inputs inputs, float dTime)
	{
		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->dActive)
				collectibles[index]->DUpdate(game, this, frameCount, inputs, dTime);

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->dActive)
				sortedNCEntities[index]->DUpdate(game, this, frameCount, inputs, dTime);
	}

	void UIUpdate(Game* game, int frameCount, Inputs inputs, float dTime)
	{
		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->dActive && collectibles[index]->shouldUI)
				collectibles[index]->UIUpdate(game, this, frameCount, inputs, dTime);

		for (index = 0; index < sortedNCEntities.size(); index++)
			if (sortedNCEntities[index]->dActive && sortedNCEntities[index]->shouldUI)
				sortedNCEntities[index]->UIUpdate(game, this, frameCount, inputs, dTime);
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

void Item::OnDeath(Entities* entities, Vec2 pos, Entity* creator)
{
	entities->push_back(new Collectible(*this, pos));
}

void Entity::DestroySelf(Entities* entities, Entity* damageDealer)
{
	entities->Remove(this);
	OnDeath(entities, damageDealer);
	if (holder != nullptr)
		holder->heldEntity = nullptr;
	delete this;
}

bool Entity::TryMove(Vec2 direction, int force, Entities* entities, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force >= mass && direction != Vec2(0, 0))
	{
		vector<Entity*> overlaps = entities->FindCorpOverlaps(newPos, dimensions);
		for(Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != entity->creator || creator == nullptr) &&
				!entity->TryMove(direction, force - mass, entities, ignore) && !entity->Overlaps(pos, dimensions))
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
	else return false;

	pos = newPos;
	return true;
}

bool Entity::TryMove(Vec2 direction, int force, Entities* entities, Entity** hitEntity, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force >= mass && direction != Vec2(0, 0))
	{
		vector<Entity*> overlaps = entities->FindCorpOverlaps(newPos, dimensions);
		for(Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != entity->creator || creator == nullptr))
			{
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

	pos = newPos;
	return true;
}

bool Entity::CheckMove(Vec2 direction, int force, Entities* entities, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force >= mass && direction != Vec2(0, 0))
	{
		vector<Entity*> overlaps = entities->FindCorpOverlaps(newPos, dimensions);
		for (Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != entity->creator || creator == nullptr) &&
				!entity->TryMove(direction, force - mass, entities, ignore) && !entity->Overlaps(pos, dimensions))
			{
				return false; // The entity is not stuck inside another entity and are blocked.
			}
	}
	else return false;

	if (holder != nullptr)
		return holder->CheckMove(direction, force - holder->mass, entities, ignore);

	return true;
}

bool Entity::CheckMove(Vec2 direction, int force, Entities* entities, Entity** hitEntity, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force >= mass && direction != Vec2(0, 0))
	{
		vector<Entity*> overlaps = entities->FindCorpOverlaps(newPos, dimensions);
		for (Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != entity->creator || creator == nullptr))
			{
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

	return true;
}

#pragma endregion


// Post entities definition entities:

class ExplodeNextFrame : public Entity
{
public:
	int damage;
	Vec2 explosionDimensions;
	float startTime;

	ExplodeNextFrame(int damage = 1, Vec2 explosionDimensions = vOne, Vec2 pos = vZero) :
		Entity(pos), damage(damage), explosionDimensions(explosionDimensions), startTime(tTime) { }


	void Update(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		if (tTime != startTime)
		{
			vector<Entity*> hitEntities = entities->FindCorpOverlaps(pos, explosionDimensions);
			for (Entity* entity : hitEntities)
				if (entity != this)
					entity->DealDamage(damage, entities, nullptr);
			DestroySelf(entities, this);
		}
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

	PlacedOnLanding(Entity* entityToPlace, int damage = 0, int count = 1) :
		Item(entityToPlace->name, entityToPlace->color, damage, count), entityToPlace(entityToPlace) { }

	PlacedOnLanding(Entity* entityToPlace, string name, Color color = olc::MAGENTA, int damage = 1, int count = 1) :
		Item(name, color, damage, count), entityToPlace(entityToPlace) { }

	PlacedOnLanding(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", Color color = olc::MAGENTA, int damage = 1, int count = 1) :
		Item(baseClass, name, color, damage, count), entityToPlace(entityToPlace) { }

	Item Clone(int count) override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, color, damage, count);
	}

	Item Clone() override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, color, damage, count);
	}

	Item* Clone2(int count) override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, color, damage, count);
	}

	Item* Clone2() override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, color, damage, count);
	}

	void OnDeath(Entities* entities, Vec2 pos, Entity* creator) override
	{
		entities->push_back(entityToPlace->Clone(pos));
	}
};

class ExplodeOnLanding : public Item
{
public:
	Vec2 explosionDimensions;

	ExplodeOnLanding(Vec2 explosionDimensions = vOne, string name = "NULL", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f) :
		Item(name, color, damage, count), explosionDimensions(explosionDimensions) { }

	ExplodeOnLanding(Item* baseClass, Vec2 explosionDimensions = vOne, string name = "NULL", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f) :
		Item(baseClass, name, color, damage, count), explosionDimensions(explosionDimensions) { }

	virtual Item Clone(int count)
	{
		return ExplodeOnLanding(baseClass, explosionDimensions, name, color, damage, count, range);
	}

	virtual Item Clone()
	{
		return ExplodeOnLanding(baseClass, explosionDimensions, name, color, damage, count, range);
	}

	virtual Item* Clone2(int count)
	{
		return new ExplodeOnLanding(baseClass, explosionDimensions, name, color, damage, count, range);
	}

	virtual Item* Clone2()
	{
		return new ExplodeOnLanding(baseClass, explosionDimensions, name, color, damage, count, range);
	}

	void OnDeath(Entities* entities, Vec2 pos, Entity* creator) override
	{
		vector<Entity*> hitEntities = entities->FindCorpOverlaps(pos, explosionDimensions);
		for (Entity* entity : hitEntities)
			if (entity != creator)
				entity->DealDamage(damage, entities, nullptr);
		entities->push_back(new FadeOut(0.5f, pos, explosionDimensions, color));
	}
};

namespace Resources
{
	ExplodeOnLanding* ruby = new ExplodeOnLanding(vOne * 2, "Ruby", Color(168, 50, 100), 2);
	ExplodeOnLanding* emerald = new ExplodeOnLanding(vOne * 4, "Emerald", Color(65, 224, 150), 1);
}

namespace Collectibles
{
	Collectible* ruby = new Collectible(*Resources::ruby, vZero);
	Collectible* emerald = new Collectible(*Resources::emerald, vZero);
}


typedef pair<Cost, Item*> RecipeA;
typedef pair<Cost, Entity*> RecipeB;