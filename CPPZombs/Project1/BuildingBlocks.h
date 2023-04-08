#include "Projectile.h"

class DToCol : public Entity
{
public:
	RGBA color2;

	DToCol(Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, radius, color, mass, maxHealth, health, name), color2(color2)
	{
		vUpdate = VUPDATE::FRICTION;
		dUpdate = DUPDATE::DTOCOL;
	}
};

namespace DUpdates
{
	void DToColDU(Entity* entity)
	{
		DToCol* dToCol = static_cast<DToCol*>(entity);
		float t = (float)dToCol->health / (float)dToCol->maxHealth;
		RGBA tempColor = dToCol->color;
		dToCol->color = RGBA(int(dToCol->color2.r + t * (dToCol->color.r - dToCol->color2.r)),
			int(dToCol->color2.g + t * (dToCol->color.g - dToCol->color2.g)),
			int(dToCol->color2.b + t * (dToCol->color.b - dToCol->color2.b)),
			int(dToCol->color2.a + t * (dToCol->color.a - dToCol->color2.a)));
		dToCol->DUpdate(DUPDATE::ENTITY);
		dToCol->color = tempColor;
	}
}

class LightBlock : public DToCol
{
public:
	JRGB lightColor;
	float range;
	LightSource* lightSource;
	bool lightOrDark; // If dark then it'll subtract it true then it'll add.

	LightBlock(JRGB lightColor, bool lightOrDark, float range = 50, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		RGBA color2 = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(pos, radius, color, color2, mass, maxHealth, health, name), lightColor(lightColor),
		range(range), lightSource(nullptr), lightOrDark(lightOrDark)
	{
		onDeath = ONDEATH::LIGHTBLOCK;
	}

	void Start() override
	{
		unique_ptr<LightSource> sharedPtr = make_unique<LightSource>(pos, lightColor, range);
		lightSource = sharedPtr.get();
		if (lightOrDark)
			game->entities->lightSources.push_back(std::move(sharedPtr));
		else
			game->entities->darkSources.push_back(std::move(sharedPtr));
	}

	LightBlock(LightBlock* baseClass, Vec3 pos) :
		LightBlock(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<LightBlock>(this, pos);
	}

	void SetPos(Vec3 newPos) override
	{
		DToCol::SetPos(newPos);
		lightSource->pos = pos;
	}
};

namespace OnDeaths {
	void LightBlockOD(Entity* entity, Entity* damageDealer)
	{
		LightBlock* light = static_cast<LightBlock*>(entity);
		if (light->lightOrDark)
			game->entities->RemoveLight(light->lightSource);
		else
			game->entities->RemoveDark(light->lightSource);
	}
}

namespace Shootables
{
	LightBlock* cheese = new LightBlock({ 255, 255, 0 }, true, 25, vZero, 0.5f, RGBA(235, 178, 56), RGBA(0, 0, 0, 127), 1, 1, 1, "Cheese");
	LightBlock* shades = new LightBlock({ 255, 255, 255 }, false, 15, vZero, 0.5f, RGBA(255, 255, 255), RGBA(), 1, 1, 1, "Shades");
}

namespace Resources
{
	PlacedOnLanding* cheese = new PlacedOnLanding(Shootables::cheese, "Cheese", "Light", 3, Shootables::cheese->color, 0);
	PlacedOnLanding* shades = new PlacedOnLanding(Shootables::shades, "Shades", "Light", 3, Shootables::shades->color, 0);
}

namespace Collectibles
{
	Collectible* cheese = new Collectible(*Resources::cheese);
	Collectible* shades = new Collectible(*Resources::shades);
}

enum class TUPDATE
{
	DEFAULT, TREE, VINE
};

vector<function<bool(Entity*)>> tUpdates;

class FunctionalBlock : public Entity
{
public:
	TUPDATE tUpdate;
	float timePer, lastTime;

	FunctionalBlock(float timePer, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime), Entity(pos, radius, color, mass, maxHealth, health, name), tUpdate(TUPDATE::DEFAULT)
	{
		update = UPDATE::FUNCTIONALBLOCK;
		vUpdate = VUPDATE::FRICTION;
		Start();
	}

	FunctionalBlock(float timePer, float offset, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime + offset), Entity(pos, radius, color, mass, maxHealth, health, name), tUpdate(TUPDATE::DEFAULT)
	{
		update = UPDATE::FUNCTIONALBLOCK;
		Start();
	}

	FunctionalBlock() = default;

	bool TUpdate()
	{
		return tUpdates[UnEnum(tUpdate)](this);
	}
	bool TUpdate(TUPDATE tempTUpdate)
	{
		return tUpdates[UnEnum(tempTUpdate)](this);
	}
};

class FunctionalBlock2 : public Entity // Can have speed multipliers.
{
public:
	TUPDATE tUpdate;
	float timePer, timeSince;

	FunctionalBlock2(float timePer, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0), Entity(pos, radius, color, mass, maxHealth, health, name), tUpdate(TUPDATE::DEFAULT)
	{
		update = UPDATE::FUNCTIONALBLOCK2;
		vUpdate = VUPDATE::FRICTION;
		Start();
	}

	FunctionalBlock2(float timePer, float offset, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0 + offset), Entity(pos, radius, color, mass, maxHealth, health, name), tUpdate(TUPDATE::DEFAULT)
	{
		update = UPDATE::FUNCTIONALBLOCK2;
		Start();
	}

	FunctionalBlock2() = default;

	virtual float TimeIncrease()
	{
		return game->dTime;
	}

	bool TUpdate()
	{
		return tUpdates[UnEnum(tUpdate)](this);
	}
	bool TUpdate(TUPDATE tempTUpdate)
	{
		return tUpdates[UnEnum(tempTUpdate)](this);
	}
};

namespace Updates
{
	void FunctionalBlockU(Entity* entity)
	{
		FunctionalBlock* block = static_cast<FunctionalBlock*>(entity);
		if (tTime - block->lastTime >= block->timePer)
		{
			if (block->TUpdate())
				block->lastTime = tTime;
		}
	}

	void FunctionalBlock2U(Entity* entity)
	{
		FunctionalBlock2* block = static_cast<FunctionalBlock2*>(entity);
		block->timeSince += block->TimeIncrease();
		if (block->timeSince >= block->timePer)
		{
			if (block->TUpdate())
				block->timeSince -= block->timePer;
		}
	}
}

namespace TUpdates { bool DefaultTU(Entity* entity) { return true; } }