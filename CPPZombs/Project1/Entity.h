#include "Item.h"

#define COMMON_TEXT_SCALE trueScreenHeight / 20
#define COMMON_BOARDER_WIDTH trueScreenHeight / 80

#pragma region Psuedo-virtual functions
enum class UPDATE // Update
{
	ENTITY, FADEOUT, EXPLODENEXTFRAME, UPEXPLODENEXTFRAME, FADEOUTPUDDLE, VACUUMEFOR, PROJECTILE, FUNCTIONALBLOCK, FUNCTIONALBLOCK2, ENEMY, POUNCERSNAKE, VACUUMER, SPIDER,
	CENTICRAWLER, POUNCER, CAT, CATACLYSM, EGG, KIWI, PLAYER, TURRET, ROVER, ENGINEER, GRENADE
};

vector<function<void(Entity*)>> updates;

enum class VUPDATE // Update
{
	ENTITY, FRICTION
};

vector<function<void(Entity*)>> vUpdates;

enum class DUPDATE // Draw Update
{
	ENTITY, FADEOUT, FADEOUTPUDDLE, FADEOUTGLOW, DTOCOL, SHRUB, TREE, DECEIVER, PARENT, EXPLODER, SNAKECONNECTED, COLORCYCLER,
	POUNCER, CAT, CATACLYSM, TANK, KIWI, PLAYER, TURRET, ROVER
};

vector<function<void(Entity*)>> dUpdates;

enum class EDUPDATE // Early Draw Update
{
	ENTITY, SNAKE, SNAKECONNECTED, SPIDER
};

vector<function<void(Entity*)>> eDUpdates;

enum class UIUPDATE // User-Interface Update
{
	ENTITY, SHRUB, TREE, VINE, ENEMY, SNAKECONNECTED, PLAYER, ENGINEER
};

vector<function<void(Entity*)>> uiUpdates;

enum class ONDEATH
{
	ENTITY, FADEOUTGLOW, SHOTITEM, LIGHTBLOCK, VINE, ENEMY, PARENT, EXPLODER, SNAKE, POUNCERSNAKE, SNAKECONNECTED, VACUUMER, SPIDER,
	CENTICRAWLER, KIWI, PLAYER
};

vector<function<void(Entity*, Entity*)>> onDeaths;
#pragma endregion

class TimedEvent
{
public:
	float startTime;
	std::function<bool(void* entity, TimedEvent* mEvent)> function;

	TimedEvent(std::function<bool(void* entity, TimedEvent* mEvent)> function) :
		function(function), startTime(tTime) { }
};

class Entities;
class Entity
{
public:
	UPDATE update;
	VUPDATE vUpdate;
	DUPDATE dUpdate;
	EDUPDATE earlyDUpdate;
	UIUPDATE uiUpdate;
	ONDEATH onDeath;
	Entity* baseClass;
	Entity* creator;
	vector<Entity*> observers{};
	string name;
	Vec3 pos, lastPos, vel, dir;
	float radius;
	RGBA color;
	float mass;
	int maxHealth, health;
	bool active = true, dActive = true, uiActive = false;
	int sortLayer = 0;
	float foodValue = 0; // If 0 then is not food.
	bool isLight = true, canAttack = true, isEnemy = false, isProjectile = false, isCollectible = false, corporeal = true;

	Entity(Vec3 pos = Vec3(0), float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), lastPos(pos), vel(0), dir(0), radius(radius), color(color),
		mass(mass), maxHealth(maxHealth), health(health), name(name), baseClass(this), creator(nullptr),
		update(UPDATE::ENTITY), vUpdate(VUPDATE::ENTITY), dUpdate(DUPDATE::ENTITY), earlyDUpdate(EDUPDATE::ENTITY),
		uiUpdate(UIUPDATE::ENTITY), onDeath(ONDEATH::ENTITY)
	{
	}

	Entity(Entity* baseClass, Vec3 pos):
		Entity(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	virtual unique_ptr<Entity> Clone(Vec3 pos = Vec3(0), Vec3 dir = Vec3(0, 1, 0), Entity* creator = nullptr)
	{
		return make_unique<Entity>(this, pos);
	}

	virtual void Start() { }

	void Draw(Vec3 pos)
	{
		Vec3 tempPos = this->pos;
		this->pos = pos;
		DUpdate();
		this->pos = tempPos;
	}

#pragma region Psuedo-virtual functions
	void Update() // Normally doesn't draw.
	{
		updates[UnEnum(update)](this);
	}
	void Update(UPDATE tempUpdate) // Normally doesn't draw.
	{
		updates[UnEnum(tempUpdate)](this);
	}

	void VUpdate() // Normally just increases position by velocity and modifies velocity.
	{
		vUpdates[UnEnum(vUpdate)](this);
	}
	void VUpdate(VUPDATE tempVUpdate) // Normally just increases position by velocity and modifies velocity.
	{
		vUpdates[UnEnum(tempVUpdate)](this);
	}

	void DUpdate() // Normally only draws. ALSO, this only calls the function, it is not the actual DUpdate function, for that look in the global DUpdate namespace.
	{
		dUpdates[UnEnum(dUpdate)](this);
	}
	void DUpdate(DUPDATE tempDUpdate) // Normally only draws. ALSO, this only calls the function, it is not the actual DUpdate function, for that look in the global DUpdate namespace.
	{
		dUpdates[UnEnum(tempDUpdate)](this);
	}

	void EarlyDUpdate() // Does nothing by default, used by weird rendering systems like the mighty spoobderb.
	{
		eDUpdates[UnEnum(earlyDUpdate)](this);
	}
	void EarlyDUpdate(EDUPDATE tempEDUpdate) // Does nothing by default, used by weird rendering systems like the mighty spoobderb.
	{
		eDUpdates[UnEnum(tempEDUpdate)](this);
	}

	void UIUpdate() // Draws when uiActive is true.
	{
		uiUpdates[UnEnum(uiUpdate)](this);
	}
	void UIUpdate(UIUPDATE tempUIUpdate) // Draws when uiActive is true.
	{
		uiUpdates[UnEnum(tempUIUpdate)](this);
	}

	void OnDeath(Entity* damageDealer) // Called whenever an entity has died, damage dealer is often nullptr.
	{
		onDeaths[UnEnum(onDeath)](this, damageDealer);
	}
	void OnDeath(ONDEATH tempOnDeath, Entity* damageDealer) // Called whenever an entity has died, damage dealer is often nullptr.
	{
		onDeaths[UnEnum(tempOnDeath)](this, damageDealer);
	}
#pragma endregion

	Vec2 BottomLeft() // Not always accurate.
	{
		return (pos - game->PlayerPos() + Vec3(radius, -radius, -radius)) * (trueScreenHeight / game->zoom);
	}

	void DrawUIBox(Vec2 bottomLeft, Vec2 topRight, float boarderWidth, string text, RGBA textColor,
		RGBA borderColor = RGBA(127, 127, 127), RGBA fillColor = RGBA(63, 63, 63))
	{
		game->DrawFBL(bottomLeft, borderColor, topRight - bottomLeft + boarderWidth);
		game->DrawFBL(bottomLeft + boarderWidth, fillColor, topRight - bottomLeft); // Draw the middle box, +1.
		font.Render(text, bottomLeft + boarderWidth + south2 * (font.mininumVertOffset / 2.f), static_cast<float>(COMMON_TEXT_SCALE), textColor);
	}

	virtual void SetPos(Vec3 newPos);
	virtual void SetRadius(float newRadius);

	virtual int ApplyHit(int damage, Entity* damageDealer);

	void DestroySelf(Entity* damageDealer); // Always calls OnDeath;
	void DelayedDestroySelf(); // Never calls OnDeath;

	virtual bool Overlaps(Vec3 pos, float radius)
	{
		return glm::length2(this->pos - pos) < (this->radius + radius) * (this->radius + radius);
	}

	void UpdateChunkCollision();
	void UpdateCollision();

	virtual void UnAttach(Entity* entity) { }
};




#pragma region Other Entity funcitons

namespace MaskF
{
	bool IsCorporeal(Entity* from, Entity* to)
	{
		return from != to && to->corporeal;
	}

	bool IsCorporealNotCollectible(Entity* from, Entity* to)
	{
		return from != to && to->corporeal && !to->isCollectible;
	}

	bool IsCorporealNotCreator(Entity* from, Entity* to)
	{
		return from != to && from->creator != to && to->corporeal && !to->isCollectible;
	}

	bool IsEnemy(Entity* from, Entity* to)
	{
		return  to->isEnemy && from != to && to->corporeal && !to->isCollectible && to != from->creator;
	}

	bool IsNonEnemy(Entity* from, Entity* to)
	{
		return !to->isEnemy && from != to && to->corporeal && !to->isCollectible;
	}

	bool IsSameType(Entity* from, Entity* to)
	{
		return to->baseClass == from->baseClass && from != to && to->corporeal;
	}
}

bool BoxCircleOverlap(Vec3 cPos, float radius, Vec3 bPos, Vec3 bDim)
{
	Vec3 difference = cPos - bPos;
	Vec3 clamped = glm::clamp(difference, -bDim, bDim);
	// add clamped value to AABB_center and we get the value of box closest to circle
	Vec3 closest = bPos + clamped;
	// retrieve vector between center circle and closest point AABB and check if length <= radius
	difference = closest - cPos;
	return glm::length2(difference) < radius * radius;
}

vector<Entity*> EntitiesOverlaps(Vec3 pos, float radius, vector<Entity*> entities)
{
	vector<Entity*> foundEntities(0);
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->Overlaps(pos, radius))
			foundEntities.push_back(*i);
	return foundEntities;
}

#pragma endregion



class FadeOut : public Entity
{
public:
	float startTime, totalFadeTime;

	FadeOut(float totalFadeTime = 1.0f, Vec3 pos = Vec3(0), float radius = 0.5f, RGBA color = RGBA()) :
		Entity(pos, radius, color), totalFadeTime(totalFadeTime), startTime(tTime)
	{
		update = UPDATE::FADEOUT;
		dUpdate = DUPDATE::FADEOUT;
		corporeal = false;
	}

	float Opacity()
	{
		return 0.1f * roundf((1.0f - (tTime - startTime) / totalFadeTime) * 10);
	}
};

namespace Updates
{
	void EntityU(Entity* entity) { }

	void FadeOutU(Entity* entity)
	{
		if (tTime - ((FadeOut*)entity)->startTime > ((FadeOut*)entity)->totalFadeTime)
			entity->DestroySelf(entity);
	}
}

namespace VUpdates
{
	void EntityVU(Entity* entity)
	{
		entity->lastPos = entity->pos;
		entity->SetPos(entity->pos + entity->vel * game->dTime);
	}

	void FrictionVU(Entity* entity)
	{
		entity->lastPos = entity->pos;
		entity->SetPos(entity->pos + entity->vel * game->dTime);
		entity->vel = Vec3(FromTo2(entity->vel, vZero, game->dTime * game->planet->friction), (entity->vel.z - game->planet->gravity * game->dTime) * powf(0.5f, game->dTime));
	}
}

namespace DUpdates
{
	void EntityDU(Entity* entity) // Normally only draws.
	{
		game->DrawCircle(entity->pos, entity->color, entity->radius);
	}

	void FadeOutDU(Entity* entity)
	{
		entity->color.a = static_cast<uint8_t>(((FadeOut*)entity)->Opacity() * 255);
		glDepthMask(GL_FALSE);
		entity->DUpdate(DUPDATE::ENTITY);
		glDepthMask(GL_TRUE);
	}
}

namespace EDUpdates { void EntityEDU(Entity* entity) { } }

namespace UIUpdates
{
	void EntityUIU(Entity* entity)
	{
		Vec2 bottomLeft = entity->BottomLeft();
		Vec2 topRight = bottomLeft + Vec2(font.TextWidth(entity->name) * COMMON_TEXT_SCALE / font.minimumSize, font.maxVertOffset / 2) * 0.5f;
		entity->DrawUIBox(bottomLeft, topRight, static_cast<float>(COMMON_BOARDER_WIDTH), entity->name, entity->color);
	}
}

namespace OnDeaths { void EntityOD(Entity* entity, Entity* damageDealer) { } }

namespace OverlapRes
{
	void CircleOR(Entity* a, Entity* b)
	{
		float dist = glm::length(b->pos - a->pos);

		// Compute unit normal and unit tangent vectors
		Vec3 v_un = Normalized(b->pos - a->pos); // unit normal vector

		// Compute scalar projections of velocities onto v_un and v_ut
		float v1n = glm::dot(v_un, a->vel); // Dot product
		float v2n = glm::dot(v_un, b->vel);

		// Set new velocities in x and y coordinates
		a->vel = v_un * (v1n * (a->mass - b->mass) + 2.f * b->mass * v2n) / (a->mass + b->mass);
		b->vel = v_un * (v2n * (b->mass - a->mass) + 2.f * a->mass * v1n) / (b->mass + a->mass);

		Vec3 multiplier = (b->pos - a->pos) * (1.01f * (dist - a->radius - b->radius) / (dist * (a->mass + b->mass)));
		a->SetPos(a->pos + multiplier * b->mass);
		b->SetPos(b->pos - multiplier * a->mass);
	}
}