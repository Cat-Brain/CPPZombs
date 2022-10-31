#include "Entity.h"

class DToCol : public Entity
{
public:
	Color color2;

	DToCol(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), int mass = 1, int maxHealth = 1, int health = 1) :
		Entity(pos, color, mass, maxHealth, health),
		color2(color2)
	{ }

	void Update(olc::PixelGameEngine* screen, vector<Entity*> entities, int frameCount, Inputs inputs) override
	{
		float t = (float)health / (float)maxHealth;
		screen->Draw(pos.x, screenHeight - pos.y - 1, Color(color2.r + t * (color.r - color2.r), color2.g + t * (color.g - color2.g), color2.b + t * (color.b - color2.b), color2.a + t * (color.a - color2.a)));
	}
};

class FunctionalBlock : Entity
{
public:
	using Entity::Entity;
	int ticksPerActivate = 1;

	void Update(olc::PixelGameEngine* screen, vector<Entity*> entities, int frameCount, Inputs inputs) override
	{
		if (frameCount % ticksPerActivate == 0)
			TUpdate(screen, entities, frameCount, inputs);

		Entity::Update(screen, entities, frameCount, inputs);
	}

	virtual void TUpdate(olc::PixelGameEngine* screen, vector<Entity*> entities, int frameCount, Inputs inputs)
	{

	}
};