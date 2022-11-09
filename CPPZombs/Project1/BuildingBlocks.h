#include "Entity.h"

class MiniEntity : public Entity
{
public:
	MiniEntity(Vec2 pos, Color color) :
		Entity(pos, color)
	{ }
	
	void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		screen->Draw(ToRSpace(pos), color);
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

	void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		float t = (float)health / (float)maxHealth;
		Color tempColor = color;
		color = Color(color2.r + t * (color.r - color2.r), color2.g + t * (color.g - color2.g), color2.b + t * (color.b - color2.b), color2.a + t * (color.a - color2.a));
		Entity::Update(screen, entities, frameCount, inputs);
		color = tempColor;
	}
};

class Placeable : public DToCol
{
public:
	using DToCol::DToCol;

	void OnDeath(vector<Entity*>* entities) override
	{
		entities->push_back(new MiniEntity(pos, color));
	}
};

class FunctionalBlock : public Entity
{
public:
	using Entity::Entity;

	void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		if (frameCount % TickPer() == 0)
			TUpdate(screen, (Entities*)entities, frameCount, inputs);

		Entity::Update(screen, entities, frameCount, inputs);
	}

	virtual int TickPer()
	{
		return 2;
	}

	virtual void TUpdate(olc::PixelGameEngine* screen, Entities* entities, int frameCount, Inputs inputs) { }
};

/*class Conveyer : public FunctionalBlock
{
public:
	virtual int MaxStack()
	{
		return 3;
	}

	int TickPer() override
	{
		return 3;
	}

	void TUpdate(olc::PixelGameEngine* screen, Entities* entities, int frameCount, Inputs inputs) override
	{
		vector<Conveyer*> nearbyFilledConveyers = vector<Conveyer*>();
		vector<Conveyer*> nearbyEmptyConveyers = vector<Conveyer*>();

		for (int i = 0; i < entities->conveyers.size(); i++)
			if (Squistance(pos, entities->conveyers[i]->pos) == 1)
			{
				if (entities->conveyers[i]->containedEntities.size() < ((Conveyer*)entities->conveyers[i])->MaxStack())
				{
					if (containedEntities.size() != 0)
						nearbyEmptyConveyers.push_back((Conveyer*)entities->conveyers[i]);
				}
				else
					nearbyFilledConveyers.push_back((Conveyer*)entities->conveyers[i]);
			}

		for (int i = 0; i < nearbyEmptyConveyers.size(); i++)
		{
			int countToDis = fminf(ceilf(containedEntities.size() / nearbyEmptyConveyers.size()), nearbyEmptyConveyers[i]->MaxStack() - nearbyEmptyConveyers[i]->containedEntities.size());
			for (int j = 0; j < countToDis; j++)
			{
				nearbyEmptyConveyers[i]->containedEntities.push_back(containedEntities.back());
				containedEntities.pop_back();
			}
		}

		for (int i = 0; i < nearbyFilledConveyers.size() && containedEntities.size(); i++)
		{
			int countToDis = fminf(nearbyFilledConveyers[i]->containedEntities.size(), )
			for(int j = 0; j <)
		}
	}
};*/ 