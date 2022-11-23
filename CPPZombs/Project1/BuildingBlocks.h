#include "Projectile.h"

class DToCol : public Entity
{
public:
	Color color2;

	DToCol(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), Recipe cost = Recipes::dRecipe, int mass = 1, int maxHealth = 1, int health = 1) :
		Entity(pos, color, cost, mass, maxHealth, health), color2(color2)
	{ }

	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		float t = (float)health / (float)maxHealth;
		Color tempColor = color;
		color = Color(color2.r + t * (color.r - color2.r), color2.g + t * (color.g - color2.g), color2.b + t * (color.b - color2.b), color2.a + t * (color.a - color2.a));
		Entity::DUpdate(screen, entities, frameCount, inputs);
		color = tempColor;
	}
};

class Placeable : public DToCol
{
public:
	using DToCol::DToCol;

	Placeable(Placeable* baseClass, Vec2 pos) :
		Placeable(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	Placeable() {}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return new Placeable(this, pos);
	}

	void OnDeath(vector<Entity*>* entities) override
	{
		if (rand() % 2)
			for (int i = 0; i < cost.size(); i++)
				((Entities*)entities)->push_back(new Collectible(&cost[i], ToRandomCSpace(pos)));
	}
};
Placeable* copperWall = new Placeable(Vec2(0, 0), olc::YELLOW, Color(0, 0, 0, 127), Recipes::copperWall, 1, 4, 4);

class FunctionalBlock : public Entity
{
public:
	int tickPer;

	FunctionalBlock(int tickPer, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Recipe cost = Recipes::dRecipe, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		tickPer(tickPer), Entity(pos, color, cost, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock() = default;

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		if (frameCount % tickPer == 0)
			TUpdate(screen, (Entities*)entities, frameCount, inputs);
	}

	virtual void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs) { }
};

class OffsettedFunctionalBlock : public FunctionalBlock
{
public:
	using FunctionalBlock::FunctionalBlock;
	int offset;

	void Start() override
	{
		offset = frameCount;
	}

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		if ((frameCount + offset) % tickPer == 0)
			TUpdate(screen, (Entities*)entities, frameCount, inputs);
	}
};

class Duct : public FunctionalBlock
{
public:
	Vec2 dir;
	vector<Collectible*> newlyCollected;

	Duct(Vec2 dir, int tickPer, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Recipe cost = Recipes::dRecipe, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		dir(dir), newlyCollected(), FunctionalBlock(tickPer, pos, color, cost, mass, maxHealth, health, name)
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

	void Draw(Vec2 pos, Color color, Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, Vec2 dir = vZero) override
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		Color tempColor = this->color;
		this->color = color;
		Vec2 tempDir = this->dir;
		this->dir = dir;
		DUpdate(screen, entities, frameCount, inputs);
		this->pos = tempPos;
		this->color = tempColor;
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

	void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs)
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

	void OnDeath(vector<Entity*>* entities) override
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
Duct* duct = new Duct(up, 2, vZero, olc::GREEN, Recipes::conveyer);