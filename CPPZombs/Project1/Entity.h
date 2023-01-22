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
	Vec2 pos, dir;
	Vec2 dimensions; // Actually half dimensions. So a 3x3 would actually be called a 2x2, and a 1x1 would remain a 1x1.
	Color color;
	int mass;
	int maxHealth, health;
	bool active = true, dActive = true;

	Entity(Vec2 pos = Vec2(0, 0), Vec2 dimensions = Vec2(1, 1), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		pos(pos), dimensions(dimensions), dir(0, 0), color(color), mass(mass), maxHealth(maxHealth), health(health), name(name), baseClass(this), creator(nullptr)
	{
	}

	Entity(Entity* baseClass, Vec2 pos):
		Entity(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	virtual Entity* Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr)
	{
		return new Entity(this, pos);
	}

	virtual void Start() { }

	virtual void Draw(Vec2 pos, Color color, Vec2 dir = vZero)
	{
		Vec2 tempPos = this->pos;
		this->pos = pos;
		Color tempColor = this->color;
		this->color = color;
		DUpdate();
		this->pos = tempPos;
		this->color = tempColor;
	}

	virtual void DUpdate() // Normally only draws.
	{
		Vec2 disp = ToRSpace(pos);
		disp = Vec2(labs(disp.x), labs(disp.y));
		if(disp.x >= 0 && disp.x <= game->ScreenWidth() && disp.y >= 0 && disp.y <= game->ScreenWidth())
			game->FillRect(ToRSpace(pos) - dimensions + vOne, dimensions * 2 - vOne, color);
	}

	void DrawUIBox(Vec2 topLeft, Vec2 bottomRight, string text, Color textColor,
		Color borderColor = olc::VERY_DARK_GREY, Color fillColor = olc::DARK_GREY)
	{
		game->DrawRect(topLeft, bottomRight - topLeft, borderColor);
		game->FillRect(topLeft + Vec2(1, 1), bottomRight - topLeft - Vec2(1, 1), fillColor);
		game->DrawString(topLeft + Vec2(1, 1), text, textColor);
	}

	virtual Vec2 TopLeft()
	{
		return ToRSpace(pos + dimensions - vOne) * 4 + Vec2(4, 0);
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
	
	virtual bool TryMove(Vec2 direction, int force = 1, Entity* ignore = nullptr, Entity** hitEntity = nullptr); // returns if item was hit.
	virtual void SetPos(Vec2 newPos);

	virtual int DealDamage(int damage, Entity* damageDealer);

	void DestroySelf(Entity* damageDealer); // Always calls OnDeath;

	virtual void OnDeath(Entity* damageDealer) { }

	virtual int SortOrder()
	{
		return 0;
	}

	virtual bool Overlaps(Vec2 pos, Vec2 hDim)
	{
		return labs(this->pos.x - pos.x) < (dimensions.x + hDim.x) - 1 && labs(this->pos.y - pos.y) < (dimensions.y + hDim.y) - 1;
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
bool EmptyFromEntities(Vec2 pos, vector<Entity*> entities)
{
	for (int i = 0; i < entities.size(); i++)
		if (entities[i]->pos == pos)
			return false;
	return true;
}

vector<Entity*> IncorporealsAtPos(Vec2 pos, vector<Entity*>* entities)
{
	vector<Entity*> foundEntities = vector<Entity*>();
	for (vector<Entity*>::iterator i = entities->begin(); i != entities->end(); i++)
		if (!(*i)->Corporeal() && (*i)->pos == pos)
			foundEntities.push_back(*i);
	return foundEntities;
}

vector<Entity*> CorporealsAtPos(Vec2 pos, vector<Entity*>* entities)
{
	vector<Entity*> foundEntities = vector<Entity*>();
	for (vector<Entity*>::iterator i = entities->begin(); i != entities->end(); i++)
		if ((*i)->Corporeal() && (*i)->pos == pos)
			foundEntities.push_back(*i);
	return foundEntities;
}
#pragma endregion



class FadeOut : public Entity
{
public:
	float startTime, totalFadeTime;

	FadeOut(float totalFadeTime = 1.0f, Vec2 pos = Vec2(0, 0), Vec2 dimensions = Vec2(1, 1), Color color = Color(olc::WHITE)) :
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



class LightSource
{
public:
	Vec2 pos;
	Color color;
	int falloff;

	LightSource(Vec2 pos = Vec2(0, 0), Color color = olc::WHITE, int falloff = 50) :
		pos(pos), color(color), falloff(falloff) { }

	void ApplyLight()
	{
		Vec2 camMin = playerPos - screenDimH, camMax = playerPos + screenDimH;
		int maxDistance = ceilf(max(color.r, max(color.g, color.b)) / (float)falloff);
		Vec2 lightMin = pos - vOne * maxDistance, lightMax = pos + vOne * maxDistance;
		for (int x = max(camMin.x, lightMin.x); x < min(camMax.x, lightMax.x); x++)
			for (int y = max(camMin.y, lightMin.y); y < min(camMax.y, lightMax.y); y++)
			{
				int falloffAmount = falloff * Diagnistance(pos, Vec2(x, y));
				game->shadowMap[x - camMin.x][y - camMin.y][0] += Clamp(color.r - falloffAmount, 0, 255 - game->shadowMap[x - camMin.x][y - camMin.y][0]);
				game->shadowMap[x - camMin.x][y - camMin.y][1] += Clamp(color.g - falloffAmount, 0, 255 - game->shadowMap[x - camMin.x][y - camMin.y][1]);
				game->shadowMap[x - camMin.x][y - camMin.y][2] += Clamp(color.b - falloffAmount, 0, 255 - game->shadowMap[x - camMin.x][y - camMin.y][2]);
			}
	}

	/*Color GetEffect(Vec2 position) // In world.
	{
		return color * min(1.0f, intensity / Diagnistance(position, pos));
	}*/
};


typedef pair<Cost, Item*> RecipeA;
typedef pair<Cost, Entity*> RecipeB;