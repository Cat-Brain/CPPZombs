#include "Item.h"

#define COMMON_TEXT_SCALE trueScreenHeight / 20
#define COMMON_BOARDER_WIDTH trueScreenHeight / 80

#pragma region Psuedo-virtual functions
enum UPDATE // Update
{
	ENTITYU, FADEOUTU, EXPLODENEXTFRAMEU, FADEOUTPUDDLEU, PROJECTILEU, FUNCTIONALBLOCKU, FUNCTIONALBLOCK2U, ENEMYU, POUNCERSNAKEU, VACUUMERU, SPIDERU,
	CENTICRAWLERU, POUNCERU, CATACLYSMU, PLAYERU
};

vector<function<void(Entity*)>> updates;

enum DUPDATE // Draw Update
{
	ENTITYDU, FADEOUTDU, FADEOUTPUDDLEDU, FADEOUTGLOWDU, DTOCOLDU, TREEDU, DECEIVERDU, PARENTDU, EXPLODERDU, COLORCYCLERDU, CATDU, CATACLYSMDU
};

vector<function<void(Entity*)>> dUpdates;

enum EDUPDATE // Early Draw Update
{
	ENTITYEDU
};

vector<function<void(Entity*)>> eDUpdates;

enum UIUPDATE // User-Interface Update
{
	ENTITYUIU, TREEUIU, VINEUIU, ENEMYUIU
};

vector<function<void(Entity*)>> uiUpdates;

enum OVERLAPFUN
{
	ENTITYOF
};

vector<function<bool(Entity*, Vec2, float)>> overlapFuns;

enum OVERLAPRES // Overlap resolutions
{
	CIRCLE
};

vector<function<void(Entity*, Entity*)>> overlapRes; // Overlap resolutions.

enum ONDEATH
{
	ENTITYOD, FADEOUTGLOWOD, SHOTITEMOD, LIGHTBLOCKOD, VINEOD, ENEMYOD, PARENTOD, EXPLODEROD, SNAKEOD, POUNCERSNAKEOD, VACUUMEROD, SPIDEROD,
	CENTICRAWLEROD, PLAYEROD
};

vector<function<void(Entity*, Entity*)>> onDeaths;
#pragma endregion

class Entities;
class Entity
{
public:
	UPDATE update;
	DUPDATE dUpdate;
	EDUPDATE earlyDUpdate;
	UIUPDATE uiUpdate;
	ONDEATH onDeath;
	OVERLAPFUN overlapFun;
	bool shouldUI = false;
	Entity* baseClass;
	Entity* creator;
	Entity* holder = nullptr, *heldEntity = nullptr;
	string name;
	Vec2 pos, dir;
	float radius;
	RGBA color, subScat;
	float mass;
	int maxHealth, health;
	bool active = true, dActive = true;
	int sortLayer = 0;
	bool isLight = true, canAttack = true, isEnemy = false, isProjectile = false, isCollectible = false, corporeal = true;

	Entity(Vec2 pos = Vec2(0), float radius = 0.5f, RGBA color = RGBA(), RGBA subScat = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), radius(radius), dir(0), color(color), subScat(subScat),
		mass(mass), maxHealth(maxHealth), health(health), name(name), baseClass(this), creator(nullptr),
		update(UPDATE::ENTITYU), dUpdate(DUPDATE::ENTITYDU), earlyDUpdate(EDUPDATE::ENTITYEDU), uiUpdate(UIUPDATE::ENTITYUIU), onDeath(ONDEATH::ENTITYOD), overlapFun(OVERLAPFUN::ENTITYOF)
	{
	}

	Entity(Entity* baseClass, Vec2 pos):
		Entity(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	virtual unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr)
	{
		return make_unique<Entity>(this, pos);
	}

	virtual void Start() { }

	void Draw(Vec2 pos)
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		DUpdate();
		this->pos = tempPos;
	}

#pragma region Psuedo-virtual functions
	void Update() // Normally doesn't draw.
	{
		updates[update](this);
	}
	void Update(UPDATE tempUpdate) // Normally doesn't draw.
	{
		updates[tempUpdate](this);
	}

	void DUpdate() // Normally only draws. ALSO, this only calls the function, it is not the actual DUpdate function, for that look in the global DUpdate namespace.
	{
		dUpdates[dUpdate](this);
	}
	void DUpdate(DUPDATE tempDUpdate) // Normally only draws. ALSO, this only calls the function, it is not the actual DUpdate function, for that look in the global DUpdate namespace.
	{
		dUpdates[tempDUpdate](this);
	}

	void EarlyDUpdate() // Does nothing by default, used by weird rendering systems like the mighty spoobderb.
	{
		eDUpdates[earlyDUpdate](this);
	}
	void EarlyDUpdate(EDUPDATE tempEDUpdate) // Does nothing by default, used by weird rendering systems like the mighty spoobderb.
	{
		eDUpdates[tempEDUpdate](this);
	}

	void UIUpdate() // Draws when shouldUI is true.
	{
		uiUpdates[uiUpdate](this);
	}
	void UIUpdate(UIUPDATE tempUIUpdate) // Draws when shouldUI is true.
	{
		uiUpdates[tempUIUpdate](this);
	}

	void OnDeath(Entity* damageDealer) // Called whenever an entity has died, damage dealer is often nullptr.
	{
		onDeaths[onDeath](this, damageDealer);
	}
	void OnDeath(ONDEATH tempOnDeath, Entity* damageDealer) // Called whenever an entity has died, damage dealer is often nullptr.
	{
		onDeaths[tempOnDeath](this, damageDealer);
	}
#pragma endregion

	virtual void SubScatUpdate() // Renders the sub-surface scattering of the entity.
	{
		//game->Draw(pos, subScat, radius);
	}

	Vec2 BottomLeft() // Not always accurate.
	{
		return (pos - game->PlayerPos() + Vec2(1, -1) * radius) / game->zoom * Vec2(ScrDim()) * Vec2(1, 1);
	}

	void DrawUIBox(Vec2 bottomLeft, Vec2 topRight, float boarderWidth, string text, RGBA textColor,
		RGBA borderColor = RGBA(127, 127, 127), RGBA fillColor = RGBA(63, 63, 63))
	{
		game->DrawFBL(bottomLeft, borderColor, topRight - bottomLeft + boarderWidth);
		game->DrawFBL(bottomLeft + boarderWidth, fillColor, topRight - bottomLeft); // Draw the middle box, +1.
		font.Render(text, bottomLeft + boarderWidth + down * (font.mininumVertOffset / 2.f), static_cast<float>(COMMON_TEXT_SCALE), textColor);
	}

	virtual void SetPos(Vec2 newPos);

	virtual bool TryMove(Vec2 direction, float force, Entity* ignore = nullptr, Entity** hitEntity = nullptr); // returns index of hit item.

	virtual int DealDamage(int damage, Entity* damageDealer);

	void DestroySelf(Entity* damageDealer); // Always calls OnDeath;

	bool Overlaps(Vec2 pos, float radius)
	{
		return overlapFuns[overlapFun](this, pos, radius);
	}

	void UpdateCollision();
};




#pragma region Other Entity funcitons

vector<Entity*> EntitiesOverlaps(Vec2 pos, float radius, vector<Entity*> entities)
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

	FadeOut(float totalFadeTime = 1.0f, iVec2 pos = vZero, float radius = 0.5f, RGBA color = RGBA()) :
		Entity(pos, radius, color), totalFadeTime(totalFadeTime), startTime(tTime)
	{
		update = UPDATE::FADEOUTU;
		dUpdate = DUPDATE::FADEOUTDU;
		corporeal = false;
	}

	float Opacity()
	{
		return 1.0f - (tTime - startTime) / totalFadeTime;
	}
};

namespace Updates
{
	void EntityU(Entity* entity) { /*printf("Test test");*/ }

	void FadeOutU(Entity* entity)
	{
		if (tTime - ((FadeOut*)entity)->startTime > ((FadeOut*)entity)->totalFadeTime)
			entity->DestroySelf(entity);
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
		entity->DUpdate(DUPDATE::ENTITYDU);
	}
}

namespace EDUpdates { void EntityEDU(Entity* entity) { } }

namespace UIUpdates
{
	void EntityUIU(Entity* entity)
	{
		Vec2 bottomLeft = entity->BottomLeft();
		Vec2 topRight = bottomLeft + Vec2(font.TextWidth(entity->name) * COMMON_TEXT_SCALE / font.minimumSize, font.maxVertOffset / 2) * 0.5f;
		entity->DrawUIBox(bottomLeft, topRight, COMMON_BOARDER_WIDTH, entity->name, entity->color);
	}
}

namespace OnDeaths { void EntityOD(Entity* entity, Entity* damageDealer) { } }

namespace OverlapFuns
{
	bool EntityOF(Entity* entity, Vec2 pos, float radius)
	{
		return glm::length2(entity->pos - pos) < (entity->radius + radius) * (entity->radius + radius);
		//return labs(entity->pos.x - pos.x) < float(entity->dimensions.x + dimensions.x) / 2 && labs(entity->pos.y - pos.y) < float(entity->dimensions.y + dimensions.y) / 2;
	}
}

namespace OverlapRes
{
	void CircleOR(Entity* a, Entity* b)
	{
		float dist = glm::length(b->pos - a->pos);

		Vec2 multiplier = (b->pos - a->pos) * (1.1f * (dist - a->radius - b->radius) / (dist * (a->mass + b->mass)));
		a->SetPos(a->pos + multiplier * b->mass);
		b->SetPos(b->pos - multiplier * a->mass);
	}
}


Vec2 Game::PlayerPos()
{
	return ((Entity*)player)->pos;
}