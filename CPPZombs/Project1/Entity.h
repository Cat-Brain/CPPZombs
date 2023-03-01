#include "Item.h"

#define COMMON_TEXT_SCALE trueScreenHeight / 20
#define COMMON_BOARDER_WIDTH trueScreenHeight / 80

enum UPDATE // Update
{
	ENTITYU, FADEOUTU, EXPLODENEXTFRAMEU, FADEOUTPUDDLEU, PROJECTILEU, FUNCTIONALBLOCKU, FUNCTIONALBLOCK2U, ENEMYU, POUNCERSNAKEU, SPIDERU,
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
	ENTITYEDU, SPIDEREDU
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

vector<function<bool(Entity*, iVec2, iVec2)>> overlapFuns;

enum ONDEATH
{
	ENTITYOD, FADEOUTGLOWOD, SHOTITEMOD, LIGHTBLOCKOD, VINEOD, ENEMYOD, PARENTOD, EXPLODEROD, SNAKEOD, POUNCERSNAKEOD, VACUUMEROD, SPIDEROD,
	CENTICRAWLEROD, PLAYEROD
};

vector<function<void(Entity*, Entity*)>> onDeaths;

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
	iVec2 pos, dir;
	iVec2 dimensions;
	RGBA color, subScat;
	float mass;
	int maxHealth, health;
	bool active = true, dActive = true;
	int sortLayer = 0;
	bool isLight = true, canAttack = true, isEnemy = false, isProjectile = false, isCollectible = false, corporeal = true;

	Entity(iVec2 pos = vZero, iVec2 dimensions = vOne, RGBA color = RGBA(), RGBA subScat = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), dimensions(dimensions), dir(0, 0), color(color), subScat(subScat),
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

	virtual unique_ptr<Entity> Clone(iVec2 pos = vZero, iVec2 dir = up, Entity* creator = nullptr)
	{
		return make_unique<Entity>(this, pos);
	}

	virtual void Start() { }

	void Draw(iVec2 pos, iVec2 dir = vZero)
	{
		iVec2 tempPos = this->pos;
		this->pos = pos;
		DUpdate();
		this->pos = tempPos;
	}

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

	virtual void SubScatUpdate() // Renders the sub-surface scattering of the entity.
	{
		game->Draw(pos, subScat, dimensions);
	}

	iVec2 BottomLeft() // Not always accurate.
	{
		return (pos + right * (dimensions.x / 2 + 1) - game->PlayerPos() + down * (dimensions.y / 2)) * int(2 * trueScreenHeight / midRes.height);
	}

	void DrawUIBox(iVec2 bottomLeft, iVec2 topRight, int boarderWidth, string text, RGBA textColor,
		RGBA borderColor = RGBA(127, 127, 127), RGBA fillColor = RGBA(63, 63, 63))
	{
		game->DrawFBL(bottomLeft, borderColor, topRight - bottomLeft + boarderWidth); // + 2 * boarder is to avoid clipping to avoid overlapping the text.
		game->DrawFBL(bottomLeft + boarderWidth, fillColor, topRight - bottomLeft); // Draw the middle box, +1.
		font.Render(text, bottomLeft + boarderWidth + down * (font.mininumVertOffset / 2), static_cast<float>(COMMON_TEXT_SCALE), textColor);
	}

	virtual void SetPos(iVec2 newPos);

	virtual bool TryMove(iVec2 direction, float force, Entity* ignore = nullptr, Entity** hitEntity = nullptr); // returns index of hit item.

	virtual int DealDamage(int damage, Entity* damageDealer);

	void DestroySelf(Entity* damageDealer); // Always calls OnDeath;

	bool Overlaps(iVec2 pos, iVec2 dimensions)
	{
		return overlapFuns[overlapFun](this, pos, dimensions);
	}
};




#pragma region Other Entity funcitons

vector<Entity*> EntitiesOverlaps(iVec2 pos, iVec2 dimensions, vector<Entity*> entities)
{
	vector<Entity*> foundEntities(0);
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->Overlaps(pos, dimensions))
			foundEntities.push_back(*i);
	return foundEntities;
}

#pragma endregion



class FadeOut : public Entity
{
public:
	float startTime, totalFadeTime;

	FadeOut(float totalFadeTime = 1.0f, iVec2 pos = vZero, iVec2 dimensions = vOne, RGBA color = RGBA()) :
		Entity(pos, dimensions, color), totalFadeTime(totalFadeTime), startTime(tTime)
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
		game->Draw(entity->pos, entity->color, entity->dimensions);
		game->DrawCircle(Vec2(entity->pos) + Vec2(0.5f, 0.5f), entity->color, entity->dimensions.x);
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
		iVec2 bottomLeft = entity->BottomLeft();
		iVec2 topRight = bottomLeft + iVec2(font.TextWidth(entity->name) * COMMON_TEXT_SCALE / font.minimumSize, font.maxVertOffset / 2) / 2;
		entity->DrawUIBox(bottomLeft, topRight, COMMON_BOARDER_WIDTH, entity->name, entity->color);
	}
}

namespace OnDeaths { void EntityOD(Entity* entity, Entity* damageDealer) { } }

namespace OverlapFuns
{
	bool EntityOF(Entity* entity, iVec2 pos, iVec2 dimensions)
	{
		return labs(entity->pos.x - pos.x) < float(entity->dimensions.x + dimensions.x) / 2 && labs(entity->pos.y - pos.y) < float(entity->dimensions.y + dimensions.y) / 2;
	}
}


iVec2 Game::PlayerPos()
{
	return ((Entity*)player)->pos;
}