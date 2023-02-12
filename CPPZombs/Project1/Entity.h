#include "Item.h"

#define COMMON_TEXT_SCALE trueScreenHeight / 20
#define COMMON_BOARDER_WIDTH trueScreenHeight / 80

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
	RGBA color, subScat;
	float mass;
	int maxHealth, health;
	bool active = true, dActive = true;

	Entity(Vec2 pos = 0, Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA subScat = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), dimensions(dimensions), dir(0, 0), color(color), subScat(subScat),
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

	virtual void EarlyDUpdate() { } // Does nothing by default, used by weird rendering systems like the mighty spoobderb.

	virtual void DUpdate() // Normally only draws.
	{
		game->Draw(pos, color, dimensions);
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

	virtual void UIUpdate() // Draws when shouldUI is true.
	{
		DrawUIBox(BottomLeft(), TopRight(), COMMON_BOARDER_WIDTH, name, color);
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
		Entity(pos, dimensions, color), totalFadeTime(totalFadeTime), startTime(tTime) { }

	void Update() override
	{
		if (tTime - startTime > totalFadeTime)
			DestroySelf(nullptr);
	}

	float Opacity()
	{
		return 1.0f - (tTime - startTime) / totalFadeTime;
	}

	void DUpdate() override
	{
		color.a = static_cast<uint8_t>(Opacity() * 255);
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