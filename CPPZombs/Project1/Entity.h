#include "Item.h"

class Entities;
class Entity
{
public:
	bool shouldUI = false;
	Entity* baseClass;
	Entity* creator;
	Entity* holder = nullptr, *heldEntity = nullptr;
	string name;
	Vec2 pos, dir, vel;
	Vec2 iPos;
	Vec2 dimensions;
	RGBA color, subsurfaceResistance;
	float mass;
	int maxHealth, health;
	bool active = true, dActive = true;

	Entity(Vec2 pos = Vec2(0, 0), Vec2 dimensions = Vec2(1, 1), RGBA color = RGBA(), RGBA subsurfaceResistance = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), iPos(pos), dimensions(dimensions), vel(0, 0), dir(0, 0), color(color), subsurfaceResistance(subsurfaceResistance),
		mass(mass), maxHealth(maxHealth), health(health), name(name), baseClass(this), creator(nullptr)
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

	virtual void Draw(Vec2 pos, Vec2 dir = vZero)
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		DUpdate();
		this->pos = tempPos;
	}

	virtual void EarlyDUpdate() { } // Does nothing by default, used by weird rendering systems like the mighty spoobster.

	virtual void VUpdate();

	virtual void ReduceVel()
	{
		vel *= powf(0.25f, game->dTime);
	}

	virtual void AddForce(Vec2 force)
	{
		vel += force / mass;
	}

	void ResolveCollision(Entity* other) // SUPER INCOMPLETE, DO NOT USE!
	{
		Vec2 p = pos - other->pos;
		Vec2 b = dimensions + other->dimensions;
		Vec2 w = p.Abs() - b;
		Vec2 s = Vec2(p.x < 0.0 ? -1 : 1, p.y < 0.0 ? -1 : 1);
		float g = max(w.x, w.y);
		Vec2  q = w.V2fMin(0.0);
		float l = q.Magnitude();
		Vec2 movement = s * ((g > 0.0) ? q / l : ((w.x > w.y) ? Vec2(1, 0) : Vec2(0, 1)));
	}

	virtual void DUpdate() // Normally only draws.
	{
		game->Draw(iPos, color, dimensions);
	}

	void DrawUIBox(Vec2 topLeft, Vec2 bottomRight, string text, RGBA textColor,
		RGBA borderColor = RGBA(127, 127, 127), RGBA fillColor = RGBA(63, 63, 63))
	{
		game->DrawFBL(topLeft, borderColor, bottomRight - topLeft);
		game->DrawFBL(topLeft + Vec2(1, 1), fillColor, bottomRight - topLeft - Vec2(1, 1));
		//game->DrawString(topLeft + Vec2(1, 1), text, textColor);
	}

	virtual Vec2 TopLeft()
	{
		return pos + dimensions - vOne * 4 + Vec2(4, 0);
	}

	virtual Vec2 BottomRight()
	{
		return TopLeft() + Vec2((int)name.length() * 8, 8);
	}

	virtual void UIUpdate() // Draws when shouldUI is true.
	{
		DrawUIBox(TopLeft(), BottomRight(), name, color);
	}

	virtual bool PosInUIBounds(Vec2 screenSpacePos)
	{
		Vec2 topLeft = TopLeft();
		Vec2 bottomRight = BottomRight();
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}

	virtual void Update() { } // Normally doesn't draw.
	
	virtual void SetPos(Vec2 newPos);

	virtual int DealDamage(int damage, Entity* damageDealer);

	void DestroySelf(Entity* damageDealer); // Always calls OnDeath;

	virtual void OnDeath(Entity* damageDealer) { }

	virtual int SortOrder()
	{
		return 0;
	}

	virtual bool IOverlaps(Vec2 iPos, Vec2 dim)
	{
		return labs(this->iPos.x - iPos.x) < (dimensions.x + dim.x) / 2 && labs(this->iPos.y - iPos.y) < (dimensions.y + dim.y) / 2;
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

vector<Entity*> EntitiesOverlaps(Vec2 iPos, Vec2 dimensions, vector<Entity*> entities)
{
	vector<Entity*> foundEntities(0);
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->IOverlaps(iPos, dimensions))
			foundEntities.push_back(*i);
	return foundEntities;
}
#pragma endregion



class FadeOut : public Entity
{
public:
	float startTime, totalFadeTime;

	FadeOut(float totalFadeTime = 1.0f, Vec2 pos = Vec2(0, 0), Vec2 dimensions = Vec2(1, 1), RGBA color = RGBA()) :
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

iVec2 Game::IPlayerPos()
{
	return ((Entity*)player)->iPos;
}