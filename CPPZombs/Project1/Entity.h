#include "Item.h"

#define COMMON_TEXT_SCALE trueScreenHeight / 20
#define COMMON_BOARDER_WIDTH trueScreenHeight / 80

enum UPDATE
{
	ENTITY, FADEOUT, EXPLODENEXTFRAME, FADEOUTPUDDLE, PROJECTILE, FUNCTIONALBLOCK, FUNCTIONALBLOCK2, ENEMY, POUNCERSNAKE, SPIDER, POUNCER
};

vector<function<void(Entity*)>> updates;

enum DUPDATE
{
	ENTITY, FADEOUT, FADEOUTPUDDLE, FADEOUTGLOW, DTOCOL, TREE, DECEIVER, PARENT, EXPLODER, COLORCYCLER, CAT
};

vector<function<void(Entity*)>> dUpdates;

enum EDUPDATE // EDUPDATE = early dupdate = early draw update
{
	ENTITY, SPIDER
};

vector<function<void(Entity*)>> eDUpdates;

enum UIUPDATE
{
	ENTITY, TREE, VINE
};

vector<function<void(Entity*)>> uiUpdates;

class Entities;
class Entity
{
public:
	UPDATE update;
	DUPDATE dUpdate;
	EDUPDATE earlyDUpdate;
	UIUPDATE uiUpdate;
	bool shouldUI = false;
	Entity* baseClass;
	Entity* creator;
	Entity* holder = nullptr, *heldEntity = nullptr;
	string name;
	Vec2 pos, dir;
	Vec2 dimensions;
	RGBA color, subScat;
	float mass;
	int maxHealth, health;
	bool active = true, dActive = true;

	Entity(Vec2 pos = 0, Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA subScat = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), dimensions(dimensions), dir(0, 0), color(color), subScat(subScat),
		mass(mass), maxHealth(maxHealth), health(health), name(name), baseClass(this), creator(nullptr),
		update(UPDATE::ENTITY), dUpdate(DUPDATE::ENTITY), earlyDUpdate(EDUPDATE::ENTITY), uiUpdate(UIUPDATE::ENTITY)
	{
	}

	Entity(Entity* baseClass, Vec2f pos):
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

	void Draw(Vec2 pos, Vec2 dir = vZero)
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		DUpdate();
		this->pos = tempPos;
	}

	void Update() // Normally doesn't draw.
	{
		std::bind(&updates[update], this);
		updates[update](this);
	}
	void Update(UPDATE tempUpdate) // Normally doesn't draw.
	{
		std::bind(&updates[tempUpdate], this);
		updates[tempUpdate](this);
	}

	void DUpdate() // Normally only draws. ALSO, this only calls the function, it is not the actual DUpdate function, for that look in the global DUpdate namespace.
	{
		std::bind(&dUpdates[dUpdate], this);
		dUpdates[dUpdate](this);
	}
	void DUpdate(DUPDATE tempDUpdate) // Normally only draws. ALSO, this only calls the function, it is not the actual DUpdate function, for that look in the global DUpdate namespace.
	{
		std::bind(&dUpdates[tempDUpdate], this);
		dUpdates[tempDUpdate](this);
	}

	void EarlyDUpdate() // Does nothing by default, used by weird rendering systems like the mighty spoobderb.
	{
		std::bind(&eDUpdates[earlyDUpdate], this);
		eDUpdates[earlyDUpdate](this);
	}
	void EarlyDUpdate(EDUPDATE tempEDUpdate) // Does nothing by default, used by weird rendering systems like the mighty spoobderb.
	{
		std::bind(&eDUpdates[tempEDUpdate], this);
		eDUpdates[tempEDUpdate](this);
	}

	void UIUpdate() // Draws when shouldUI is true.
	{
		std::bind(&uiUpdates[uiUpdate], this);
		uiUpdates[uiUpdate](this);
	}
	void UIUpdate(UIUPDATE tempUIUpdate) // Draws when shouldUI is true.
	{
		std::bind(&uiUpdates[tempUIUpdate], this);
		uiUpdates[tempUIUpdate](this);
	}

	virtual void SubScatUpdate() // Renders the sub-surface scattering of the entity.
	{
		game->Draw(pos, subScat, dimensions);
	}

	void DrawUIBox(Vec2 bottomLeft, Vec2 topRight, int boarderWidth, string text, RGBA textColor,
		RGBA borderColor = RGBA(127, 127, 127), RGBA fillColor = RGBA(63, 63, 63))
	{
		game->DrawFBL(bottomLeft, borderColor, topRight - bottomLeft + boarderWidth); // + 2 * boarder is to avoid clipping to avoid overlapping the text.
		game->DrawFBL(bottomLeft + boarderWidth, fillColor, topRight - bottomLeft); // Draw the middle box, +1.
		font.Render(text, bottomLeft + boarderWidth + down * (font.mininumVertOffset / 2), COMMON_TEXT_SCALE, textColor);
	}

	virtual Vec2 BottomLeft()
	{
		return (pos + right - game->PlayerPos() + dimensions / 2) * 2 * trueScreenHeight / midRes.height;
	}

	virtual Vec2 TopRight() 
	{
		return BottomLeft() + Vec2(font.TextWidth(name) * COMMON_TEXT_SCALE / font.minimumSize, font.maxVertOffset / 2) / 2;
	}

	virtual bool PosInUIBounds(Vec2 screenSpacePos)
	{
		Vec2 topLeft = BottomLeft();
		Vec2 bottomRight = TopRight();
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}
	
	virtual void SetPos(Vec2 newPos);

	virtual bool TryMove(Vec2 direction, int force, Entity* ignore = nullptr, Entity** hitEntity = nullptr); // returns index of hit item.

	virtual int DealDamage(int damage, Entity* damageDealer);

	void DestroySelf(Entity* damageDealer); // Always calls OnDeath;

	virtual void OnDeath(Entity* damageDealer) { }

	virtual int SortOrder()
	{
		return 0;
	}

	virtual bool Overlaps(Vec2 pos, Vec2 dim)
	{
		return labs(this->pos.x - pos.x) < float(dimensions.x + dim.x) / 2 && labs(this->pos.y - pos.y) < float(dimensions.y + dim.y) / 2;
	}

	#pragma region bool functions

	virtual bool IsLight()
	{
		return true;
	}

	virtual bool CanAttack()
	{
		return true;
	}

	virtual bool CanConveyer()
	{
		return false;
	}

	virtual bool IsConveyer()
	{
		return false;
	}

	virtual bool IsEnemy()
	{
		return false;
	}

	virtual bool IsProjectile()
	{
		return false;
	}

	virtual bool Corporeal()
	{
		return true;
	}

	virtual bool IsCollectible()
	{
		return false;
	}

	#pragma endregion
};




#pragma region Other Entity funcitons

vector<Entity*> EntitiesOverlaps(Vec2 pos, Vec2 dimensions, vector<Entity*> entities)
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

	FadeOut(float totalFadeTime = 1.0f, Vec2 pos = 0, Vec2 dimensions = vOne, RGBA color = RGBA()) :
		Entity(pos, dimensions, color), totalFadeTime(totalFadeTime), startTime(tTime)
	{
		update = UPDATE::FADEOUT;
		dUpdate = DUPDATE::FADEOUT;
	}

	float Opacity()
	{
		return 1.0f - (tTime - startTime) / totalFadeTime;
	}


	bool Corporeal() override
	{
		return false;
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

namespace DUpdates
{
	void EntityDU(Entity* entity) // Normally only draws.
	{
		game->Draw(entity->pos, entity->color, entity->dimensions);
	}

	void FadeOutDU(Entity* entity)
	{
		entity->color.a = static_cast<uint8_t>(((FadeOut*)entity)->Opacity() * 255);
		entity->DUpdate();
	}
}

namespace EDUpdates { void EntityEDU(Entity* entity) { } }

namespace UIUpdates
{
	void UIUpdate(Entity* entity)
	{
		entity->DrawUIBox(entity->BottomLeft(), entity->TopRight(), COMMON_BOARDER_WIDTH, entity->name, entity->color);
	}
}


Vec2 Game::PlayerPos()
{
	return ((Entity*)player)->pos;
}