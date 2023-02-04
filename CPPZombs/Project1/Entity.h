#include "Item.h"

#define COMMON_TEXT_SCALE ScrHeight() / 20

class Entities;
class Entity
{
public:
	bool shouldUI = false;
	Entity* baseClass;
	Entity* creator;
	Entity* holder = nullptr, *heldEntity = nullptr;
	string name;
	Vec2 pos, dir;
	Vec2 dimensions;
	RGBA color, subsurfaceResistance;
	float mass;
	int maxHealth, health;
	bool active = true, dActive = true;

	Entity(Vec2 pos = 0, Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA subsurfaceResistance = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), dimensions(dimensions), dir(0, 0), color(color), subsurfaceResistance(subsurfaceResistance),
		mass(mass), maxHealth(maxHealth), health(health), name(name), baseClass(this), creator(nullptr)
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

	virtual void Draw(Vec2 pos, Vec2 dir = vZero)
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		DUpdate();
		this->pos = tempPos;
	}

	virtual void EarlyDUpdate() { } // Does nothing by default, used by weird rendering systems like the mighty spoobster.

	void ResolveCollision(Entity* other) // SUPER INCOMPLETE, DO NOT USE!
	{
		Vec2f p = pos - other->pos;
		Vec2f b = dimensions + other->dimensions;
		Vec2f w = p.Abs() - b;
		Vec2f s = Vec2f(p.x < 0.0 ? -1 : 1, p.y < 0.0 ? -1 : 1);
		float g = max(w.x, w.y);
		Vec2f  q = w.V2fMin(0.0);
		float l = q.Magnitude();
		Vec2f movement = s * ((g > 0.0) ? q / l : ((w.x > w.y) ? Vec2f(1, 0) : Vec2f(0, 1)));
	}

	virtual void DUpdate() // Normally only draws.
	{
		game->Draw(pos, color, dimensions);
	}

	void DrawUIBox(Vec2 bottomLeft, Vec2 topRight, string text, RGBA textColor,
		RGBA borderColor = RGBA(127, 127, 127), RGBA fillColor = RGBA(63, 63, 63))
	{
		game->DrawFBL(bottomLeft, borderColor, topRight - bottomLeft);
		game->DrawFBL(bottomLeft + 1, fillColor, topRight - bottomLeft - 2); // Draw the middle box, +1 and -2 to avoid overlap.
		game->DrawString(text, pos + right, COMMON_TEXT_SCALE, textColor, Vec2(2, 2));
	}

	virtual Vec2 BottomLeft()
	{
		return (pos - game->PlayerPos() + dimensions / 2 + right) * 2 * ScrDim() / midRes.ScrDim();
	}

	virtual Vec2 TopRight()
	{
		return BottomLeft() + Vec2(font.TextWidth(name) * COMMON_TEXT_SCALE / font.minimumSize, COMMON_TEXT_SCALE / 2);
	}

	virtual void UIUpdate() // Draws when shouldUI is true.
	{
		DrawUIBox(BottomLeft(), TopRight(), name, color);
	}

	virtual bool PosInUIBounds(Vec2 screenSpacePos)
	{
		Vec2 topLeft = BottomLeft();
		Vec2 bottomRight = TopRight();
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}

	virtual void Update() { } // Normally doesn't draw.
	
	virtual void SetPos(Vec2 newPos);

	bool TryMove(Vec2 direction, int force, Entity* ignore = nullptr, Entity** hitEntity = nullptr); // returns index of hit item.

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
		Entity(pos, dimensions, color), totalFadeTime(totalFadeTime), startTime(tTime) { }

	void Update() override
	{
		if (tTime - startTime > totalFadeTime)
			DestroySelf(nullptr);
	}

	void DUpdate() override
	{
		color.a = 255 - static_cast<uint8_t>((tTime - startTime) * 255 / totalFadeTime);
		Entity::DUpdate();
	}

	bool Corporeal() override
	{
		return false;
	}
};


typedef std::pair<Cost, Item*> RecipeA;
typedef std::pair<Cost, Entity*> RecipeB;

Vec2 Game::PlayerPos()
{
	return ((Entity*)player)->pos;
}