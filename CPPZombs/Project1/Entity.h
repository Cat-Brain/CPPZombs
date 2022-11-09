#include "Include.h"

class Entity
{
public:
	Vec2 pos;
	Color color;
	int mass;
	int maxHealth, health;
	vector<Entity*> containedEntities;

	Entity(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1) :
		pos(pos), color(color), mass(mass), maxHealth(maxHealth), health(health), containedEntities()
	{
		Start();
	}

	virtual void Start()
	{}

	virtual void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) // Also draws.
	{
		screen->FillRect(ToRSpace(pos), Vec2(3, 3), color);
	}

	virtual bool TryMove(Vec2 direction, int force, vector<Entity*> entities, int ignore = -1) // returns index of hit item.
	{
		Vec2 newPos = pos + direction;

		if (Squagnitude(force * direction) > 0)
		{
			for (int i = 0; i < entities.size(); i++)
				if (i != ignore && entities[i]->pos == newPos)
				{
					if (!entities[i]->TryMove(direction, force - entities[i]->mass, entities, ignore))
					{
						return false;
					}
					break;
				}
		}
		else return false;

		pos = newPos;
		return true;
	}

	virtual bool TryMove(Vec2 direction, int force, vector<Entity*> entities, int* index, int ignore = -1) // returns index of hit item.
	{
		Vec2 newPos = pos + direction;

		if (Squagnitude(force * direction) > 0)
		{
			for (int i = 0; i < entities.size(); i++)
				if (i != ignore && entities[i]->pos == newPos)
				{
					*index = i;
					if (!entities[i]->TryMove(direction, force - entities[i]->mass, entities, ignore))
					{
						return false;
					}
					break;
				}
		}
		else return false;

		pos = newPos;
		return true;
	}

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

	void DestroySelf(vector<Entity*>* entities);

	void DestroySelf(vector<Entity*>::iterator position, vector<Entity*>* entities)
	{
		entities->erase(position);
		OnDeath();
		delete this;
	}

	virtual void OnDeath()
	{
	}

	virtual bool CanAttack()
	{
		return true;
	}

	virtual bool CanConveyer()
	{
		return true;
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

	static Vec2 ToSpace(Vec2 positionInWorldSpace)
	{
		return Vec2(positionInWorldSpace.x, screenHeight - positionInWorldSpace.y - 1);
	}

	static Vec2 ToRSpace(Vec2 positionInLocalSpace)
	{
		return ToSpace(positionInLocalSpace - playerPos + screenDimH) * 3;
	}
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
#pragma endregion



class Entities : public vector<Entity*>
{
protected:
	bool updatingProjectiles;
	int index;

public:
	using vector<Entity*>::vector;

	vector<Entity*> projectiles;
	vector<Entity*> nonProjectiles;
	vector<Entity*> conveyers;
	vector<Entity*> enemies;
	vector<Entity*> nonEnemies;

	void Update(olc::PixelGameEngine* screen, int frameCount, Inputs inputs)
	{
		#pragma region Conveyers
		conveyers.clear();
		for (int i = 0; i < size(); i++)
		{
			if ((*this)[i]->IsConveyer())
				conveyers.push_back((*this)[i]);
		}
		#pragma endregion
		#pragma region Enemies and Non-Enemies
		enemies.clear();
		nonEnemies.clear();
		for (int i = 0; i < size(); i++)
		{
			if ((*this)[i]->IsEnemy())
				enemies.push_back((*this)[i]);
			else
				nonEnemies.push_back((*this)[i]);
		}
		#pragma endregion
		#pragma region Projectile and Non-Projectiles
		projectiles.clear();
		nonProjectiles.clear();
		for (int i = 0; i < size(); i++)
		{
			if ((*this)[i]->IsProjectile())
				projectiles.push_back((*this)[i]);
			else
				nonProjectiles.push_back((*this)[i]);
		}
		#pragma endregion
		
		for (index = 0; index < nonProjectiles.size(); index++)
			nonProjectiles[index]->Update(screen, this, frameCount, inputs);
		for (index = 0; index < projectiles.size(); index++)
			projectiles[index]->Update(screen, this, frameCount, inputs);
	}

	void Remove(Entity* entityToRemove)
	{
		erase(std::find(begin(), end(), entityToRemove));
		if (entityToRemove->IsProjectile())
		{
			vector<Entity*>::iterator position = find(projectiles.begin(), projectiles.end(), entityToRemove);
			if(index >= distance(projectiles.begin(), position)) // distance MUST be found before erasing.
				index--;
			projectiles.erase(position);
		}
		else
		{
			vector<Entity*>::iterator position = find(nonProjectiles.begin(), nonProjectiles.end(), entityToRemove);
			if (index >= distance(nonProjectiles.begin(), position)) // distance MUST be found before erasing.
				index--;
			nonProjectiles.erase(position);
		}
	}
};

void Entity::DestroySelf(vector<Entity*>* entities)
{
	((Entities*)entities)->Remove(this);
	OnDeath();
	delete this;
}