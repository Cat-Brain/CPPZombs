#include "Projectile.h"
class DToCol : public Entity
{
public:
	RGBA color2;

	DToCol(EntityData* data, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
		float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", Allegiance allegiance = 0) :
		Entity(data, pos, radius, color, mass, bounciness, maxHealth, health, name, allegiance), color2(color2) { }

	DToCol(DToCol* baseClass, Vec3 pos, Entity* creator = nullptr) :
		DToCol(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		if (creator != nullptr)
			allegiance = creator->allegiance;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr) override
	{
		return make_unique<DToCol>(this, pos, creator);
	}

	inline RGBA Color()
	{
		return color2.CLerp(color, (float)health / (float)maxHealth);
	}
};

namespace DUpdates
{
	void DToColDU(Entity* entity)
	{
		DToCol* dToCol = static_cast<DToCol*>(entity);
		RGBA tempColor = dToCol->color;
		dToCol->color = dToCol->Color();
		DUpdates::EntityDU(entity);
		dToCol->color = tempColor;
	}
}

EntityData dToColData = EntityData(Updates::EntityU, VUpdates::FrictionVU, DUpdates::DToColDU);
class LightBlock : public DToCol
{
public:
	JRGB lightColor;
	float range;
	LightSource* lightSource;
	bool lightOrDark; // If dark then it'll subtract it true then it'll add.

	LightBlock(EntityData* data, JRGB lightColor, bool lightOrDark, float range = 50, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		RGBA color2 = RGBA(), float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", Allegiance allegiance = 0) :
		DToCol(data, pos, radius, color, color2, mass, bounciness, maxHealth, health, name, allegiance), lightColor(lightColor),
		range(range), lightSource(nullptr), lightOrDark(lightOrDark) { }

	void Start() override
	{
		unique_ptr<LightSource> sharedPtr = make_unique<LightSource>(pos, lightColor, range);
		lightSource = sharedPtr.get();
		if (lightOrDark)
			game->entities->lightSources.push_back(std::move(sharedPtr));
		else
			game->entities->darkSources.push_back(std::move(sharedPtr));
	}

	LightBlock(LightBlock* baseClass, Vec3 pos, Entity* creator = nullptr) :
		LightBlock(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		if (creator != nullptr)
			allegiance = creator->allegiance;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr) override
	{
		return make_unique<LightBlock>(this, pos, creator);
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

EntityData lightBlockData = EntityData(Updates::EntityU, VUpdates::FrictionVU, DUpdates::DToColDU, UIUpdates::EntityUIU, OnDeaths::LightBlockOD);

namespace Shootables
{
	DToCol brick = DToCol(&dToColData, vZero, 0.75, RGBA(168, 101, 81), RGBA(), 15, 0.2f, 600, 600, "Brick");

	LightBlock cheese = LightBlock(&lightBlockData, JRGB(191, 191, 61), true, 25, vZero, 0.5f, RGBA(235, 178, 56), RGBA(), 1, 0, 1, 1, "Cheese");
	LightBlock shade = LightBlock(&lightBlockData, JRGB(255, 255, 255), false, 8, vZero, 0.5f, RGBA(255, 255, 255), RGBA(), 1, 0, 300, 300, "Shades");
}

namespace Resources
{
	PlacedOnLanding brick = PlacedOnLanding(ITEMTYPE::BRICK, &Shootables::brick, "Brick", "Wall", VUpdates::GravityVU, 6, Shootables::brick.color, 0, 15, false, 0.25f, 12, 0.5f, false, true, true);
	
	PlacedOnLanding cheese = PlacedOnLanding(ITEMTYPE::CHEESE, &Shootables::cheese, "Cheese", "Light", VUpdates::GravityVU, 3, Shootables::cheese.color, 0, 15, false, 0.25f, 12, 0.5f, false, true, true);
	PlacedOnLanding shade = PlacedOnLanding(ITEMTYPE::SHADE, &Shootables::shade, "Shade", "Wall", VUpdates::GravityVU, 6, Shootables::shade.color, 0, 15, false, 0.25f, 12, 0.5f, false, true, true);
}

namespace Collectibles
{
	Collectible* brick = new Collectible(Resources::brick.Clone());

	Collectible* cheese = new Collectible(Resources::cheese.Clone());
	Collectible* shade = new Collectible(Resources::shade.Clone());
}

class FunctionalBlock : public Entity
{
public:
	float timePer, lastTime;

	FunctionalBlock(EntityData* data, float timePer, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", Allegiance allegiance = 0) :
		timePer(timePer), lastTime(tTime), Entity(data, pos, radius, color, mass, bounciness, maxHealth, health, name, allegiance)
	{
		Start();
	}

	FunctionalBlock(EntityData* data, float timePer, float offset, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), lastTime(tTime + offset), Entity(data, pos, radius, color, mass, bounciness, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock() = default;
};

class FunctionalBlock2 : public Entity // Can have speed multipliers.
{
public:
	float timePer, timeSince;

	FunctionalBlock2(EntityData* data, float timePer, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", Allegiance allegiance = 0) :
		timePer(timePer), timeSince(0), Entity(data, pos, radius, color, mass, bounciness, maxHealth, health, name, allegiance)
	{
		Start();
	}

	FunctionalBlock2(EntityData* data, float timePer, float offset, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		timePer(timePer), timeSince(0 + offset), Entity(data, pos, radius, color, mass, bounciness, maxHealth, health, name)
	{
		Start();
	}

	FunctionalBlock2() = default;

	virtual float TimeIncrease()
	{
		return game->dTime;
	}
};

namespace TUpdates { bool DefaultTU(Entity* entity) { return true; } }

class FunctionalBlockData : public EntityData
{
public:
	TUpdate tUpdate;

	FunctionalBlockData(TUpdate tUpdate = TUpdates::DefaultTU, Update update = Updates::EntityU, VUpdate vUpdate = VUpdates::EntityVU,
		DUpdate dUpdate = DUpdates::EntityDU, UIUpdate uiUpdate = UIUpdates::EntityUIU, OnDeath onDeath = OnDeaths::EntityOD) :
		EntityData(update, vUpdate, dUpdate, uiUpdate, onDeath),
		tUpdate(tUpdate) {}
};

namespace Updates
{
	void FunctionalBlockU(Entity* entity)
	{
		FunctionalBlock* block = static_cast<FunctionalBlock*>(entity);
		if (tTime - block->lastTime >= block->timePer)
		{
			if (static_cast<FunctionalBlockData*>(block->data)->tUpdate(entity))
				block->lastTime = tTime;
		}
	}

	void FunctionalBlock2U(Entity* entity)
	{
		FunctionalBlock2* block = static_cast<FunctionalBlock2*>(entity);
		block->timeSince += block->TimeIncrease();
		if (block->timeSince >= block->timePer)
		{
			if (static_cast<FunctionalBlockData*>(block->data)->tUpdate(entity))
				block->timeSince -= block->timePer;
		}
	}
}

FunctionalBlockData functionalBlockData = FunctionalBlockData(TUpdates::DefaultTU, Updates::FunctionalBlockU, VUpdates::FrictionVU);
FunctionalBlockData functionalBlock2Data = FunctionalBlockData(TUpdates::DefaultTU, Updates::FunctionalBlock2U, VUpdates::FrictionVU);

/*class Spring : public DToCol
{
public:
	float strength, dampening, restDist;

	Spring(float strength, float dampening, float restDist, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		RGBA color2 = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME", Allegiance allegiance = 0) :
		DToCol(pos, radius, color, color2, mass, maxHealth, health, name, allegiance),
		strength(strength), dampening(dampening), restDist(restDist)
	{
		vUpdate = VUPDATE::SPRING;
	}

	Spring()
	{
	}

	unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir, Entity* creator) override
	{
		return 
	}
};

namespace VUpdates
{
	void SpringVU(Entity* entity)
	{
		Spring* spring = static_cast<Spring*>(entity);
		float dist = glm::distance(spring->pos, spring->creator->pos);
		spring->vel += (spring->creator->pos - spring->pos) * ((dist - spring->restDist) * spring->strength / dist) -
			spring->vel * spring->dampening;
	}
}*/