#include "Entity.h"

class Collectible
{
public:
	Item baseClass;
	Vec2 pos;
	Color color;
	bool active;

	Collectible(Item baseClass, Vec2 pos = vZero) :
		baseClass(baseClass), pos(pos), color(baseClass.color), active(true) { }

	Collectible(Item baseClass, Vec2 pos, Color color) :
		baseClass(baseClass), pos(pos), color(color), active(true) { }

	virtual void DUpdate(Screen* screen, void* entities, int frameCount, Inputs inputs)
	{
		screen->Draw(ToRSpace(pos), color);
	}

	virtual Collectible* Clone(int count)
	{
		return new Collectible(baseClass.Clone(count), pos, color);
	}

	virtual Collectible* Clone(Vec2 pos = vZero, Vec2 dir = vZero)
	{
		return new Collectible(baseClass, pos, color);
	}

	virtual bool TryMove(Vec2 direction, int force, void* entities, void* ignore = nullptr); // returns if item was hit.
	virtual bool TryMove(Vec2 direction, int force, void* entities, void** hitEntity, void* ignore = nullptr); // returns if item was hit.

	void DestroySelf(void* entities);
};

void Item::OnDeath(vector<void*>* collectibles, vector<void*>* entities, Vec2 pos)
{
	((vector<Collectible*>*)collectibles)->push_back(new Collectible(*this, pos));
}

namespace Collectibles
{
	Collectible* copper = new Collectible(*Resources::copper, vZero);
	Collectible* iron = new Collectible(*Resources::iron, vZero);
}


#pragma region Other Collectible funcitons
vector<Collectible*> CollectiblesAtEPos(Vec2 pos, vector<Collectible*> collectibles)
{
	vector<Collectible*> foundCollectibles = vector<Collectible*>();
	for (vector<Collectible*>::iterator i = collectibles.begin(); i != collectibles.end(); i++)
		if ((*i)->pos == pos)
			foundCollectibles.push_back(*i);
	return foundCollectibles;
}

Collectible* FindACollectible(Vec2 pos, vector<Collectible*> collectibles)
{
	vector<Collectible*> foundCollectibles = vector<Collectible*>();
	for (vector<Collectible*>::iterator i = collectibles.begin(); i != collectibles.end(); i++)
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
	vector<Entity*> sortedEntities;
	vector<Entity*> projectiles, nonProjectiles;
	vector<Entity*> enemies, nonEnemies;
	vector<Entity*> corporeals, incorporeals;
	vector<Collectible*> collectibles;

	void push_back(Entity* entity)
	{
		addedEntity = true;
		vector<Entity*>::push_back(entity);
		index++;
		sortedEntities.insert(sortedEntities.begin(), entity);
		if (entity->Corporeal())
			corporeals.push_back(entity);
		else
			incorporeals.push_back(entity);
	}

	void push_back(Collectible* collectible)
	{
		collectibles.push_back(collectible);
	}

	Entity* FindNearestEnemy(Vec2 pos)
	{
		float currentBestDist = 9999.0f;
		Entity* currentBest = nullptr;
		for (Entity* entity : *this)
		{
			float dist;
			if ((dist = Distance(pos, entity->pos)) < currentBestDist)
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
			if (labs(pos.x - (*iter)->pos.x) < (hDim.x + (*iter)->dimensions.x) - 1 && labs(pos.y - (*iter)->pos.y) < (hDim.y + (*iter)->dimensions.y) - 1) overlaps.push_back(*iter);
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
		vector<EntityIndex> unsortedToSorted = vector<EntityIndex>(size());
		for (int i = 0; i < size(); i++)
			unsortedToSorted[i] = EntityIndex(i, (*this)[i]->SortOrder());
		std::sort(unsortedToSorted.begin(), unsortedToSorted.end());

		sortedEntities = vector<Entity*>(size());
		for (int i = 0; i < size(); i++)
			sortedEntities[i] = (*this)[unsortedToSorted[i].index];
	}

	void Update(Screen* screen, int frameCount, Inputs inputs, float dTime)
	{
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


		if (addedEntity)
			SortEntities();

		for (index = 0; index < sortedEntities.size(); index++)
			if (sortedEntities[index]->active)
				sortedEntities[index]->Update(screen, this, frameCount, inputs, dTime);

		if (addedEntity)
			SortEntities();
	}

	void DUpdate(Screen* screen, int frameCount, Inputs inputs, float dTime)
	{
		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->active)
				collectibles[index]->DUpdate(screen, this, frameCount, inputs);
		for (index = 0; index < sortedEntities.size(); index++)
			if (sortedEntities[index]->dActive)
				sortedEntities[index]->DUpdate(screen, this, frameCount, inputs, dTime);
	}

	void UIUpdate(Screen* screen, int frameCount, Inputs inputs, float dTime)
	{
		for (index = 0; index < sortedEntities.size(); index++)
			if (sortedEntities[index]->dActive && sortedEntities[index]->shouldUI)
				sortedEntities[index]->UIUpdate(screen, this, frameCount, inputs, dTime);
	}

	void Remove(Entity* entityToRemove)
	{
		erase(find(begin(), end(), entityToRemove));
		vector<Entity*>::iterator pos = find(sortedEntities.begin(), sortedEntities.end(), entityToRemove);
		index -= int(index >= distance(sortedEntities.begin(), pos)); // If index is past or at the position being removed then don't advance.
		sortedEntities.erase(pos);
		if (entityToRemove->Corporeal())
			corporeals.erase(find(corporeals.begin(), corporeals.end(), entityToRemove));
	}

	void Remove(Collectible* collectibleToRemove)
	{
		collectibles.erase(find(collectibles.begin(), collectibles.end(), collectibleToRemove));
	}

	void Vacuum(Vec2 pos, int vacDist)
	{
		for (Collectible* collectible : collectibles)
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
		for (Collectible* collectible : collectibles)
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

void Collectible::DestroySelf(void* entities)
{
	((Entities*)entities)->Remove(this);
	delete this;
}

void Entity::DestroySelf(vector<Entity*>* entities, Entity* damageDealer)
{
	((Entities*)entities)->Remove(this);
	OnDeath(entities, damageDealer);
	if (holder != nullptr)
		holder->heldEntity = nullptr;
	delete this;
}

bool Entity::TryMove(Vec2 direction, int force, vector<Entity*>* entities, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force > 0 && direction != Vec2(0, 0))
	{
		vector<Entity*> overlaps = ((Entities*)entities)->FindCorpOverlaps(newPos, dimensions);
		for(Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != (entity)->creator || creator == nullptr))
				if (!(entity)->TryMove(direction, force - (entity)->mass, entities, ignore))
					return false;
	}
	else return false;

	pos = newPos;
	return true;
}

bool Entity::TryMove(Vec2 direction, int force, vector<Entity*>* entities, Entity** hitEntity, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force > 0 && direction != Vec2(0, 0))
	{

		vector<Entity*> overlaps = ((Entities*)entities)->FindCorpOverlaps(newPos, dimensions);
		for (Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != (entity)->creator || creator == nullptr))
			{
				*hitEntity = entity;
				if (!entity->TryMove(direction, force - entity->mass, entities, ignore))
					return false;
			}
	}
	else return false;

	pos = newPos;
	return true;
}

bool Entity::CheckMove(Vec2 direction, int force, vector<Entity*>* entities, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force > 0 && direction != Vec2(0, 0))
	{
		vector<Entity*> overlaps = ((Entities*)entities)->FindCorpOverlaps(newPos, dimensions);
		for (Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != (entity)->creator || creator == nullptr))
				if (!entity->CheckMove(direction, force - entity->mass, entities, ignore))
					return false;
	}
	else return false;

	if (holder != nullptr)
		return holder->CheckMove(direction, force - holder->mass, entities, ignore);

	return true;
}

bool Entity::CheckMove(Vec2 direction, int force, vector<Entity*>* entities, Entity** hitEntity, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force > 0 && direction != Vec2(0, 0))
	{

		vector<Entity*> overlaps = ((Entities*)entities)->FindCorpOverlaps(newPos, dimensions);
		for (Entity* entity : overlaps)
			if (entity != ignore && (entity != this) && (creator != (entity)->creator || creator == nullptr))
			{
				*hitEntity = entity;
				if (!entity->CheckMove(direction, force - entity->mass, entities, ignore))
					return false;
			}
	}
	else return false;

	if (holder != nullptr)
		return holder->CheckMove(direction, force - holder->mass, entities, ignore);

	return true;
}

bool Collectible::TryMove(Vec2 direction, int force, void* entities, void* ignore) // returns if item was hit.
{
	Vec2 newPos = pos + direction;

	if (force > 0 && direction != Vec2(0, 0))
	{
		vector<Entity*>::iterator entity = ((Entities*)entities)->FindCorpPos(newPos);
		if (entity != ((Entities*)entities)->corporeals.end() && *entity != ignore)
			if (!(*entity)->TryMove(direction, force - (*entity)->mass, (vector<Entity*>*)entities, (Entity*)ignore))
				return false;
	}
	else return false;

	pos = newPos;
	return true;
}

bool Collectible::TryMove(Vec2 direction, int force, void* entities, void** hitEntity, void* ignore) // returns if item was hit.
{
	Vec2 newPos = pos + direction;

	if (force > 0 && direction != Vec2(0, 0))
	{
		vector<Entity*>::iterator entity = ((Entities*)entities)->FindCorpPos(newPos);
		if (entity != ((Entities*)entities)->corporeals.end() && *entity != ignore)
		{
			*hitEntity = *entity;
			if (!(*entity)->TryMove(direction, force - (*entity)->mass, (vector<Entity*>*)entities, (Entity*)ignore))
				return false;
		}
	}
	else return false;

	pos += direction * 3;
	return true;
}

#pragma endregion


// Post entity definition items:

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

	void OnDeath(vector<void*>* collectibles, vector<void*>* entities, Vec2 pos) override
	{
		((Entities*)entities)->push_back(entityToPlace->Clone(pos));
	}
};


typedef pair<Cost, Item*> RecipeA;
typedef pair<Cost, Entity*> RecipeB;