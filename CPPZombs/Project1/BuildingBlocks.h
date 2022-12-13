#include "Projectile.h"

class DToCol : public Entity
{
public:
	Color color2;

	DToCol(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, color, mass, maxHealth, health, name), color2(color2)
	{ }

	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
	{
		float t = (float)health / (float)maxHealth;
		Color tempColor = color;
		color = Color(color2.r + t * (color.r - color2.r), color2.g + t * (color.g - color2.g), color2.b + t * (color.b - color2.b), color2.a + t * (color.a - color2.a));
		Entity::DUpdate(screen, entities, frameCount, inputs, dTime);
		color = tempColor;
	}
};

class Placeable : public DToCol
{
public:
	Collectible* toPlace;

	Placeable(Collectible* toPlace, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(pos, color, color2, mass, maxHealth, health, name), toPlace(toPlace)
	{ }

	Placeable(Placeable* baseClass, Vec2 pos) :
		Placeable(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return new Placeable(this, pos);
	}

	void OnDeath(vector<Entity*>* entities, Entity* damageDealer) override
	{
		if (toPlace != nullptr && rand() % 2)
				((Entities*)entities)->push_back(toPlace->Clone(ToRandomCSpace(pos)));
	}
};

namespace Shootables
{
	Placeable* cheeseBlock = new Placeable(nullptr, Vec2(0, 0), Color(235, 178, 56), Color(0, 0, 0, 127), 1, 4, 4);
}

class FunctionalBlock : public Entity
{
public:
	float timePer, lastTime;

	FunctionalBlock(float timePer, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime), Entity(pos, color, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock(float timePer, float offset, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime + offset), Entity(pos, color, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock() = default;

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
	{
		if (tTime - lastTime >= timePer)
		{
			TUpdate(screen, (Entities*)entities, frameCount, inputs, dTime);
			lastTime = tTime;
		}
	}

	virtual void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs, float dTime) { }
};

class Duct : public FunctionalBlock
{
public:
	Vec2 dir;
	vector<Collectible*> newlyCollected;

	Duct(Vec2 dir, int timePer, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		dir(dir), newlyCollected(), FunctionalBlock(timePer, pos, color, mass, maxHealth, health, name)
	{
		Start();
	}

	Duct(Duct* baseClass, Vec2 dir, Vec2 pos) :
		Duct(*baseClass)
	{
		this->dir = dir;
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}
	
	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return new Duct(this, dir, pos);
	}

	void Draw(Vec2 pos, Color color, Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime, Vec2 dir = vZero) override
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		Color tempColor = this->color;
		//this->color = color;
		Vec2 tempDir = this->dir;
		this->dir = dir;
		DUpdate(screen, entities, frameCount, inputs);
		this->pos = tempPos;
		//this->color = tempColor;
		this->dir = tempDir;
	}

	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs)
	{
		if (newlyCollected.size() > 0)
		{
			containedCollectibles.reserve(containedCollectibles.size() + newlyCollected.size());
			containedCollectibles.insert(containedCollectibles.end(), newlyCollected.begin(), newlyCollected.end());
			newlyCollected.clear();
		}

		Vec2 rSpacePos = ToRSpace(pos);
		if (dir == up)
		{
			screen->Draw(rSpacePos + Vec2(1, 0), color); // Top
			screen->Draw(rSpacePos + Vec2(0, 1), color); // Left
			screen->Draw(rSpacePos + Vec2(2, 1), color); // Right
		}
		else if (dir == right)
		{
			screen->Draw(rSpacePos + Vec2(2, 1), color); // Right
			screen->Draw(rSpacePos + Vec2(1, 0), color); // Top
			screen->Draw(rSpacePos + Vec2(1, 2), color); // Bottom
		}
		else if (dir == down)
		{
			screen->Draw(rSpacePos + Vec2(1, 2), color); // Bottom
			screen->Draw(rSpacePos + Vec2(0, 1), color); // Left
			screen->Draw(rSpacePos + Vec2(2, 1), color); // Right
		}
		else // dir == left || dir == none of the above
		{
			screen->Draw(rSpacePos + Vec2(0, 1), color); // Left
			screen->Draw(rSpacePos + Vec2(1, 2), color); // Bottom
			screen->Draw(rSpacePos + Vec2(1, 0), color); // Top
		}
		if (containedCollectibles.size() != 0)
			screen->Draw(rSpacePos + Vec2(1, 1), containedCollectibles[0]->color);
	}

	virtual void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs)
	{
		Collectible* item = FindACollectible(pos, entities->collectibles);
		
		if (item != nullptr && item->active)
		{
			newlyCollected.push_back(item);
			item->active = false;
		}

		if (containedCollectibles.size() > 0)
		{
			Entity* entity;
			if (containedCollectibles[0]->TryMove(dir, 1, (void*)entities, (void**)&entity, nullptr))
			{
				containedCollectibles[0]->active = true;
				containedCollectibles.erase(containedCollectibles.begin());
			}
			else if (entity->IsConveyer())
			{
				entity->containedCollectibles.push_back(containedCollectibles[0]);
				containedCollectibles.erase(containedCollectibles.begin());
			}
		}
	}

	void OnDeath(vector<Entity*>* entities, Entity* damageDealer) override
	{
		for (Collectible* collectible : containedCollectibles)
			collectible->active = true;
	}

	bool TryMove(Vec2 direction, int force, vector<Entity*>* entities, Entity* ignore = nullptr) override
	{
		if (FunctionalBlock::TryMove(direction, force, entities, ignore))
		{
			for (Collectible* collectible : containedCollectibles)
				collectible->pos += direction;
			return true;
		}

		return false;
	}

	bool TryMove(Vec2 direction, int force, vector<Entity*>* entities, Entity** hitEntity, Entity* ignore = nullptr) override
	{
		return FunctionalBlock::TryMove(direction, force, entities, ignore);
	}

	bool IsConveyer() override
	{
		return true;
	}
};

class Vacuum : public Duct
{
public:
	int vacDist;

	Vacuum(int vacDist, Vec2 dir, int tickPer, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Duct(dir, tickPer, pos, color, mass, maxHealth, health, name), vacDist(vacDist)
	{ }

	Vacuum(Vacuum* baseClass, Vec2 dir, Vec2 pos) :
		Vacuum(*baseClass)
	{
		this->dir = dir;
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return new Vacuum(this, dir, pos);
	}

	void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs) override
	{
		entities->Vacuum(pos, vacDist);

		Duct::TUpdate(screen, entities, frameCount, inputs);
	}
};

namespace Structures
{
	namespace Walls
	{
		Placeable* copperWall = new Placeable(Collectibles::copper->Clone(9), Vec2(0, 0), olc::YELLOW, Color(0, 0, 0, 127), 1, 9, 9, "Copper wall");
	}

	namespace Conveyers
	{
		Duct* duct = new Duct(up, 2, vZero, olc::GREEN);
		Vacuum* smallVacuum = new Vacuum(6 * GRID_SIZE, up, 2, vZero, olc::DARK_GREEN);
		Vacuum* largeVacuum = new Vacuum(25 * GRID_SIZE, up, 2, vZero, olc::VERY_DARK_GREEN);
	}
}

namespace Shootables
{
	Item* smallVacuum = new PlacedOnLanding(Structures::Conveyers::smallVacuum, "Small vacuum", Structures::Conveyers::smallVacuum->color, 0);
	Collectible* cSmallVacuum = new Collectible(smallVacuum->Clone());
}