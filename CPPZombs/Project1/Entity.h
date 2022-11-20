#include "Collectible.h"

class Entity
{
public:
	Entity* baseClass;
	Entity* creator;
	string name;
	Vec2 pos;
	Color color;
	int mass;
	int maxHealth, health;
	vector<Collectible*> containedCollectibles;
	bool active = true;

	Entity(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), color(color), mass(mass), maxHealth(maxHealth), health(health), name(name), containedCollectibles(), baseClass(this), creator(nullptr)
	{
		Start();
	}

	Entity(Entity* baseClass, Vec2 pos):
		Entity(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	virtual Entity* Clone(Vec2 pos = vZero)
	{
		return new Entity(this, pos);
	}

	virtual void Start()
	{}

	void Draw(Vec2 pos, Color color, Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs)
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		Color tempColor = this->color;
		this->color = color;
		DUpdate(screen, entities, frameCount, inputs);
		this->pos = tempPos;
		this->color = tempColor;
	}

	virtual void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) // ONLY draws.
	{
		screen->FillRect(ToRSpace(pos), Vec2(3, 3), color);
	}

	virtual void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) // Normally doesn't draws.
	{ }

	virtual bool TryMove(Vec2 direction, int force, vector<Entity*>* entities, Entity* ignore = nullptr); // returns if item was hit.
	virtual bool TryMove(Vec2 direction, int force, vector<Entity*>* entities, Entity** hitEntity, Entity* ignore = nullptr); // returns if item was hit.


	virtual int DealDamage(int damage, vector<Entity*>* entities)
	{
		health -= damage;
		if (health <= 0)
		{
			DestroySelf(entities);
			return 1;
		}
		return 0;
	}

	void DestroySelf(vector<Entity*>* entities); // Always calls OnDeath;

	virtual void OnDeath(vector<Entity*>* entities)
	{
	}

	virtual int SortOrder()
	{
		return 0;
	}

	#pragma region bool functions
	virtual bool CanAttack()
	{
		return true;
	}

	virtual bool CanConveyer()
	{
		return false;
	}

	virtual bool IsConveyer()
	{
		return false;
	}

	virtual bool IsEnemy()
	{
		return false;
	}

	virtual bool IsProjectile()
	{
		return false;
	}

	virtual bool Corporeal()
	{
		return true;
	}
	#pragma endregion
};




#pragma region Other Entity funcitons
bool EmptyFromEntities(Vec2 pos, vector<Entity*> entities)
{
	for (int i = 0; i < entities.size(); i++)
		if (entities[i]->pos == pos)
			return false;
	return true;
}

int TryAndAttack(Vec2 pos, int damage, vector<Entity*>* entities) // Returns 0 if successful, 1 if damaged, 2 if missed.
{
	for (int i = 0; i < entities->size(); i++)
		if ((*entities)[i]->pos == pos && (*entities)[i]->CanAttack())
		{
			(*entities)[i]->health -= damage;
			if ((*entities)[i]->health < 1)
			{
				delete (*entities)[i];
				entities->erase(entities->begin() + i);
				return 0;
			}
			return 1;
		}
	return 2;
}

vector<Entity*> IncorporealsAtPos(Vec2 pos, vector<Entity*>* entities)
{
	vector<Entity*> foundEntities = vector<Entity*>();
	for (vector<Entity*>::iterator i = entities->begin(); i != entities->end(); i++)
		if (!(*i)->Corporeal() && (*i)->pos == pos)
			foundEntities.push_back(*i);
	return foundEntities;
}

vector<Entity*> CorporealsAtPos(Vec2 pos, vector<Entity*>* entities)
{
	vector<Entity*> foundEntities = vector<Entity*>();
	for (vector<Entity*>::iterator i = entities->begin(); i != entities->end(); i++)
		if ((*i)->Corporeal() && (*i)->pos == pos)
			foundEntities.push_back(*i);
	return foundEntities;
}

vector<Collectible*> CollectiblesAtEPos(Vec2 pos, vector<Collectible*> collectibles)
{
	vector<Collectible*> foundCollectibles = vector<Collectible*>();
	for (vector<Collectible*>::iterator i = collectibles.begin(); i != collectibles.end(); i++)
		if (ToESpace((*i)->pos) == pos)
			foundCollectibles.push_back(*i);
	return foundCollectibles;
}

Collectible* FindACollectible(Vec2 pos, vector<Collectible*> collectibles)
{
	vector<Collectible*> foundCollectibles = vector<Collectible*>();
	for (vector<Collectible*>::iterator i = collectibles.begin(); i != collectibles.end(); i++)
		if (ToESpace((*i)->pos) == pos)
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
	}

	void push_back(Collectible* collectible)
	{
		collectibles.push_back(collectible);
	}

	vector<Entity*>::iterator FindCorpPos(Vec2 pos)
	{
		for (vector<Entity*>::iterator iter = corporeals.begin(); iter != corporeals.end(); iter++)
			if ((*iter)->pos == pos)
				return iter;
		return corporeals.end();
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

	void Update(Screen* screen, int frameCount, Inputs inputs)
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

		addedEntity = false;
		SortEntities();

		for (index = 0; index < sortedEntities.size(); index++)
			if(sortedEntities[index]->active)
				sortedEntities[index]->Update(screen, this, frameCount, inputs);

		if (addedEntity)
			SortEntities();
	}

	void DUpdate(Screen* screen, int frameCount, Inputs inputs)
	{
		for (index = 0; index < sortedEntities.size(); index++)
			if (sortedEntities[index]->active)
				sortedEntities[index]->DUpdate(screen, this, frameCount, inputs);
		for (index = 0; index < collectibles.size(); index++)
			if (collectibles[index]->active)
				collectibles[index]->DUpdate(screen, this, frameCount, inputs);
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
};

void Collectible::DestroySelf(void* entities)
{
	((Entities*)entities)->Remove(this);
	delete this;
}

void Entity::DestroySelf(vector<Entity*>* entities)
{
	((Entities*)entities)->Remove(this);
	OnDeath(entities);
	delete this;
}

bool Entity::TryMove(Vec2 direction, int force, vector<Entity*>* entities, Entity* ignore) // returns index of hit item.
{
	Vec2 newPos = pos + direction;

	if (force > 0 && direction != Vec2(0, 0))
	{
		vector<Entity*>::iterator entity = ((Entities*)entities)->FindCorpPos(newPos);
		if (entity != ((Entities*)entities)->corporeals.end() && *entity != ignore && (creator != (*entity)->creator || creator == nullptr))
			if (!(*entity)->TryMove(direction, force - (*entity)->mass, entities, ignore))
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
		vector<Entity*>::iterator entity = ((Entities*)entities)->FindCorpPos(newPos);
		if (entity != ((Entities*)entities)->corporeals.end() && *entity != ignore && (creator != (*entity)->creator || creator == nullptr))
		{
			*hitEntity = *entity;
			if (!(*entity)->TryMove(direction, force - (*entity)->mass, entities, ignore))
				return false;
		}
	}
	else return false;

	pos = newPos;
	return true;
}

bool Collectible::TryMove(Vec2 direction, int force, void* entities, void* ignore) // returns if item was hit.
{
	Vec2 newPos = ToESpace(pos) + direction;

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
	Vec2 newPos = ToESpace(pos) + direction;

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