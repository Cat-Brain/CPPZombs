#include "Entity.h"

class MiniEntity : public Entity
{
public:
	using Entity::Entity;
	
	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		screen->Draw(ToRSpace(pos), color);
	}

	int SortOrder() override
	{
		return 2;
	}

	bool Corporeal() override
	{
		return false;
	}
};

class DToCol : public Entity
{
public:
	Color color2;

	DToCol(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), int mass = 1, int maxHealth = 1, int health = 1) :
		Entity(pos, color, mass, maxHealth, health),
		color2(color2)
	{ }

	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		float t = (float)health / (float)maxHealth;
		Color tempColor = color;
		color = Color(color2.r + t * (color.r - color2.r), color2.g + t * (color.g - color2.g), color2.b + t * (color.b - color2.b), color2.a + t * (color.a - color2.a));
		Entity::DUpdate(screen, entities, frameCount, inputs);
		color = tempColor;
	}
};

class Placeable : public DToCol
{
public:
	using DToCol::DToCol;

	Placeable(Placeable* baseClass, Vec2 pos) :
		Placeable(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	void OnDeath(vector<Entity*>* entities) override
	{
		((Entities*)entities)->push_back(new MiniEntity(baseClass, pos));
	}
};
Placeable* cheese = new Placeable(Vec2(0, 0), olc::YELLOW, Color(0, 0, 0, 127), 1, 4, 4);

class FunctionalBlock : public Entity
{
public:
	using Entity::Entity;

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		if (frameCount % TickPer() == 0)
			TUpdate(screen, (Entities*)entities, frameCount, inputs);

		Entity::Update(screen, entities, frameCount, inputs);
	}

	virtual int TickPer()
	{
		return 2;
	}

	virtual void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs) { }
};