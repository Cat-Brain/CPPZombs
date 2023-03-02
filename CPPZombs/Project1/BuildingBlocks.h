#include "Projectile.h"

class DToCol : public Entity
{
public:
	RGBA color2;

	DToCol(iVec2 pos = vZero, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
		RGBA subScat = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, radius, color, subScat, mass, maxHealth, health, name), color2(color2)
	{
		dUpdate = DUPDATE::DTOCOLDU;
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
		dToCol->DUpdate(DUPDATE::ENTITYDU);
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

	LightBlock(JRGB lightColor, bool lightOrDark, float range = 50, iVec2 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		RGBA color2 = RGBA(), RGBA subScat = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(pos, radius, color, color2, subScat, mass, maxHealth, health, name), lightColor(lightColor),
		range(range), lightSource(nullptr), lightOrDark(lightOrDark)
	{
		onDeath = ONDEATH::LIGHTBLOCKOD;
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

	LightBlock(LightBlock* baseClass, iVec2 pos) :
		LightBlock(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	unique_ptr<Entity> Clone(iVec2 pos = vZero, iVec2 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<LightBlock>(this, pos);
	}

	void SetPos(iVec2 newPos) override
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
	LightBlock* cheese = new LightBlock({ 255, 255, 0 }, true, 25, vZero, 0.5f, RGBA(235, 178, 56), RGBA(0, 0, 0, 127), RGBA(), 1, 1, 1, "Cheese");
	LightBlock* shades = new LightBlock({ 255, 255, 255 }, false, 15, vZero, 0.5f, RGBA(255, 255, 255), RGBA(), RGBA(), 1, 1, 1, "Shades");
}

namespace Resources
{
	PlacedOnLanding* cheese = new PlacedOnLanding(Shootables::cheese, "Cheese", "Light", 3, Shootables::cheese->color, Shootables::cheese->subScat, 0);
	PlacedOnLanding* shades = new PlacedOnLanding(Shootables::shades, "Shades", "Light", 3, Shootables::shades->color, Shootables::shades->subScat, 0);
}

namespace Collectibles
{
	Collectible* cheese = new Collectible(*Resources::cheese);
	Collectible* shades = new Collectible(*Resources::shades);
}

enum TUPDATE
{
	DEFAULTTU, TREETU, VINETU
};

vector<function<bool(Entity*)>> tUpdates;

class FunctionalBlock : public Entity
{
public:
	TUPDATE tUpdate;
	float timePer, lastTime;

	FunctionalBlock(float timePer, iVec2 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		RGBA subScat = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime), Entity(pos, radius, color, subScat, mass, maxHealth, health, name), tUpdate(TUPDATE::DEFAULTTU)
	{
		update = UPDATE::FUNCTIONALBLOCKU;
		Start();
	}

	FunctionalBlock(float timePer, float offset, Vec2 pos = Vec2(0, 0), float radius = 0.5f, RGBA color = RGBA(),
		RGBA subScat = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime + offset), Entity(pos, radius, color, subScat, mass, maxHealth, health, name), tUpdate(TUPDATE::DEFAULTTU)
	{
		update = UPDATE::FUNCTIONALBLOCKU;
		Start();
	}

	FunctionalBlock() = default;

	bool TUpdate()
	{
		return tUpdates[tUpdate](this);
	}
	bool TUpdate(TUPDATE tempTUpdate)
	{
		return tUpdates[tempTUpdate](this);
	}
};

class FunctionalBlock2 : public Entity // Can have speed multipliers.
{
public:
	TUPDATE tUpdate;
	float timePer, timeSince;

	FunctionalBlock2(float timePer, Vec2 pos = Vec2(0, 0), float radius = 0.5f, RGBA color = RGBA(),
		RGBA subScat = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0), Entity(pos, radius, color, subScat, mass, maxHealth, health, name), tUpdate(TUPDATE::DEFAULTTU)
	{
		update = UPDATE::FUNCTIONALBLOCK2U;
		Start();
	}

	FunctionalBlock2(float timePer, float offset, iVec2 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		RGBA subScat = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0 + offset), Entity(pos, radius, color, subScat, mass, maxHealth, health, name), tUpdate(TUPDATE::DEFAULTTU)
	{
		update = UPDATE::FUNCTIONALBLOCK2U;
		Start();
	}

	FunctionalBlock2() = default;

	virtual float TimeIncrease()
	{
		return game->dTime;
	}

	bool TUpdate()
	{
		return tUpdates[tUpdate](this);
	}
	bool TUpdate(TUPDATE tempTUpdate)
	{
		return tUpdates[tempTUpdate](this);
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