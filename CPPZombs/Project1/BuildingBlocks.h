#include "Projectile.h"

class DToCol : public Entity
{
public:
	Color color2;

	DToCol(Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK),
		Color subsurfaceResistance = olc::WHITE, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, dimensions, color, subsurfaceResistance, mass, maxHealth, health, name), color2(color2)
	{ }

	void DUpdate() override
	{
		float t = (float)health / (float)maxHealth;
		Color tempColor = color;
		color = Color(int(color2.r + t * (color.r - color2.r)), int(color2.g + t * (color.g - color2.g)), int(color2.b + t * (color.b - color2.b)), int(color2.a + t * (color.a - color2.a)));
		Entity::DUpdate();
		color = tempColor;
	}
};

class LightBlock : public DToCol
{
public:
	JRGB lightColor;
	int lightFalloff;
	LightSource* lightSource;

	LightBlock(JRGB lightColor, int lightFalloff = 50, Vec2 pos = vZero, Vec2 dimensions = vOne, Color color = Color(olc::WHITE),
		Color color2 = Color(olc::BLACK), Color subsurfaceResistance = olc::WHITE, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(pos, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name), lightColor(lightColor),
		lightFalloff(lightFalloff), lightSource(nullptr)
	{ }

	void Start() override
	{
		game->entities->lightSources.push_back(lightSource = new LightSource(pos, lightColor, lightFalloff));
	}

	LightBlock(LightBlock* baseClass, Vec2 pos) :
		LightBlock(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return new LightBlock(this, pos);
	}

	bool TryMove(Vec2 direction, int force, Entity* ignore = nullptr, Entity** hitEntity = nullptr)
	{
		if (!DToCol::TryMove(direction, force, ignore, hitEntity))
			return false;
		lightSource->pos = pos;
		return true;
	}

	void OnDeath(Entity* damageDealer) override
	{
		if (lightSource != nullptr)
		{
			game->entities->Remove(lightSource);
			delete lightSource;
		}
	}
};

namespace Shootables
{
	LightBlock* cheeseBlock = new LightBlock({ 117, 89, 28 }, 5, vZero, vOne, Color(235, 178, 56), Color(0, 0, 0, 127), olc::BLACK, 1, 4, 4, "Cheese");
}

namespace Resources
{
	PlacedOnLanding* cheese = new PlacedOnLanding(Shootables::cheeseBlock, "Cheese", "Light", Color(235, 178, 56), 0);
}

namespace Collectibles
{
	Collectible* cheese = new Collectible(*Resources::cheese);
}

class FunctionalBlock : public Entity
{
public:
	float timePer, lastTime;

	FunctionalBlock(float timePer, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE),
		Color subsurfaceResistance = olc::WHITE, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime), Entity(pos, dimensions, color, subsurfaceResistance, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock(float timePer, float offset, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE),
		Color subsurfaceResistance = olc::WHITE, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime + offset), Entity(pos, dimensions, color, subsurfaceResistance, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock() = default;

	void Update() override
	{
		if (tTime - lastTime >= timePer)
		{
			if (TUpdate())
				lastTime = tTime;
		}
	}

	virtual bool TUpdate() { return true; }
};

class FunctionalBlock2 : public Entity // Can have speed multipliers.
{
public:
	float timePer, timeSince;

	FunctionalBlock2(float timePer, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE),
		Color subsurfaceResistance = olc::WHITE, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0), Entity(pos, dimensions, color, subsurfaceResistance, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock2(float timePer, float offset, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE),
		Color subsurfaceResistance = olc::WHITE, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0 + offset), Entity(pos, dimensions, color, subsurfaceResistance, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock2() = default;

	virtual float TimeIncrease()
	{
		return game->dTime;
	}

	void Update() override
	{
		timeSince += TimeIncrease();
		if (timeSince >= timePer && TUpdate())
			timeSince -= timePer;
	}

	virtual bool TUpdate() { return true; }
};

/*
class Duct : public FunctionalBlock
{
public:
	vector<Entity*> containedEntities;
	vector<Entity*> newlyCollected;

	Duct(float timePer, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		newlyCollected(), FunctionalBlock(timePer, pos, dimensions, color, mass, maxHealth, health, name)
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
	
	Entity* Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return new Duct(this, dir, pos);
	}

	void Draw(Vec2 pos, Color color, Game* game, float dTime, Vec2 dir = vZero) override
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		Color tempColor = this->color;
		//this->color = color;
		Vec2 tempDir = this->dir;
		this->dir = dir;
		DUpdate(game, dTime);
		this->pos = tempPos;
		//this->color = tempColor;
		this->dir = tempDir;
	}

	void DUpdate(Game* game, float dTime) override
	{
		if (newlyCollected.size() > 0)
		{
			containedEntities.reserve(containedEntities.size() + newlyCollected.size());
			containedEntities.insert(containedEntities.end(), newlyCollected.begin(), newlyCollected.end());
			newlyCollected.clear();
		}

		Vec2 rSpacePos = ToRSpace(pos);
		if (dir == up)
		{
			game->Draw(rSpacePos + Vec2(1, 0), color); // Top
			game->Draw(rSpacePos + Vec2(0, 1), color); // Left
			game->Draw(rSpacePos + Vec2(2, 1), color); // Right
		}
		else if (dir == right)
		{
			game->Draw(rSpacePos + Vec2(2, 1), color); // Right
			game->Draw(rSpacePos + Vec2(1, 0), color); // Top
			game->Draw(rSpacePos + Vec2(1, 2), color); // Bottom
		}
		else if (dir == down)
		{
			game->Draw(rSpacePos + Vec2(1, 2), color); // Bottom
			game->Draw(rSpacePos + Vec2(0, 1), color); // Left
			game->Draw(rSpacePos + Vec2(2, 1), color); // Right
		}
		else // dir == left || dir == none of the above
		{
			game->Draw(rSpacePos + Vec2(0, 1), color); // Left
			game->Draw(rSpacePos + Vec2(1, 2), color); // Bottom
			game->Draw(rSpacePos + Vec2(1, 0), color); // Top
		}
		if (containedEntities.size() != 0)
			game->Draw(rSpacePos + Vec2(1, 1), containedEntities[0]->color);
	}

	bool TUpdate(Game* game, float dTime) override
	{
		Entity* entity = FindAEntity(pos, game->entities->collectibles);
		
		if (entity != nullptr && entity->active)
		{
			newlyCollected.push_back(entity);
			entity->active = false;
		}

		if (containedEntities.size() > 0)
		{
			Entity* entity;
			if (containedEntities[0]->TryMove(dir, 1, game->entities, &entity, nullptr))
			{
				containedEntities[0]->active = true;
				containedEntities.erase(containedEntities.begin());
			}
			else if (entity->IsConveyer())
			{
				((Duct*)entity)->containedEntities.push_back(containedEntities[0]);
				containedEntities.erase(containedEntities.begin());
			}
		}
		return true;
	}

	void OnDeath(Entities* entities, Entity* damageDealer) override
	{
		for (Entity* collectible : containedEntities)
			collectible->active = true;
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
	float fov;

	Vacuum(int vacDist, float fov, float timePer, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Duct(timePer, pos, dimensions, color, mass, maxHealth, health, name), vacDist(vacDist), fov(fov)
	{ }

	Vacuum(Vacuum* baseClass, Vec2 dir, Vec2 pos) :
		Vacuum(*baseClass)
	{
		this->dir = dir;
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return new Vacuum(this, dir, pos);
	}

	bool TUpdate(Game* game, float dTime) override
	{
		game->entities->VacuumCone(pos, -dir, vacDist, fov);

		return Duct::TUpdate(game, dTime);
	}
};

namespace Structures
{
	namespace Walls
	{
		Placeable* copperWall = new Placeable(Collectibles::copper->Clone(9), vZero, vOne, olc::YELLOW, Color(0, 0, 0, 127), 1, 9, 9, "Copper wall");
	}

	namespace Conveyers
	{
		Duct* duct = new Duct(0.25f, vZero, vOne, olc::GREEN);
		Vacuum* smallVacuum = new Vacuum(6, 0.75f, 0.25f, vZero, vOne, olc::DARK_GREEN, 1, 1, 1, "Small vacuum");
		Vacuum* largeVacuum = new Vacuum(50, 1.5f, 0.25f, vZero, vOne, olc::VERY_DARK_GREEN, 1, 1, 1, "Large vacuum");
	}
}

namespace Shootables
{
	PlacedOnLanding* smallVacuum = new PlacedOnLanding(Structures::Conveyers::smallVacuum, "Small vacuum", "Structure", Structures::Conveyers::smallVacuum->color, 0);
	Collectible* cSmallVacuum = new Collectible(*smallVacuum);
}*/