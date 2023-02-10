#include "Projectile.h"

class DToCol : public Entity
{
public:
	RGBA color2;

	DToCol(Vec2 pos = vZero, Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(),
		RGBA subScat = RGBA(), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, dimensions, color, subScat, mass, maxHealth, health, name), color2(color2)
	{ }

	void DUpdate() override
	{
		float t = (float)health / (float)maxHealth;
		RGBA tempColor = color;
		color = RGBA(int(color2.r + t * (color.r - color2.r)), int(color2.g + t * (color.g - color2.g)), int(color2.b + t * (color.b - color2.b)), int(color2.a + t * (color.a - color2.a)));
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

	LightBlock(JRGB lightColor, int lightFalloff = 50, Vec2 pos = vZero, Vec2 dimensions = vOne, RGBA color = RGBA(),
		RGBA color2 = RGBA(), RGBA subScat = RGBA(), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(pos, dimensions, color, color2, subScat, mass, maxHealth, health, name), lightColor(lightColor),
		lightFalloff(lightFalloff), lightSource(nullptr)
	{ }

	void Start() override
	{
		unique_ptr<LightSource> sharedPtr = make_unique<LightSource>(pos, lightColor, lightFalloff);
		lightSource = sharedPtr.get();
		game->entities->lightSources.push_back(std::move(sharedPtr));
	}

	LightBlock(LightBlock* baseClass, Vec2 pos) :
		LightBlock(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<LightBlock>(this, pos);
	}

	void SetPos(Vec2 newPos) override
	{
		DToCol::SetPos(newPos);
		lightSource->pos = pos;
	}

	void OnDeath(Entity* damageDealer) override
	{
		game->entities->Remove(lightSource);
	}
};

namespace Shootables
{
	LightBlock* cheeseBlock = new LightBlock({ 117, 89, 28 }, 5, vZero, vOne, RGBA(235, 178, 56), RGBA(0, 0, 0, 127), RGBA(25, 25, 50), 1, 4, 4, "Cheese");
}

namespace Resources
{
	PlacedOnLanding* cheese = new PlacedOnLanding(Shootables::cheeseBlock, "Cheese", "Light", Shootables::cheeseBlock->color, Shootables::cheeseBlock->subScat, 0);
}

namespace Collectibles
{
	Collectible* cheese = new Collectible(*Resources::cheese);
}

class FunctionalBlock : public Entity
{
public:
	float timePer, lastTime;

	FunctionalBlock(float timePer, Vec2 pos = vZero, Vec2 dimensions = vOne, RGBA color = RGBA(),
		RGBA subScat = RGBA(), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime), Entity(pos, dimensions, color, subScat, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock(float timePer, float offset, Vec2f pos = Vec2f(0, 0), Vec2f dimensions = vOne, RGBA color = RGBA(),
		RGBA subScat = RGBA(), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime + offset), Entity(pos, dimensions, color, subScat, mass, maxHealth, health, name)
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

	FunctionalBlock2(float timePer, Vec2f pos = Vec2f(0, 0), Vec2f dimensions = vOne, RGBA color = RGBA(),
		RGBA subScat = RGBA(), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0), Entity(pos, dimensions, color, subScat, mass, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock2(float timePer, float offset, Vec2 pos = vZero, Vec2 dimensions = vOne, RGBA color = RGBA(),
		RGBA subScat = RGBA(), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0 + offset), Entity(pos, dimensions, color, subScat, mass, maxHealth, health, name)
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