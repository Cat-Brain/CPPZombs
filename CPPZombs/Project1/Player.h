#include "Enemy.h"

class Player : public Entity
{
public:
	using Entity::Entity;

	void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		if (frameCount % 2 == 0)
		{
			Vec2 direction(0, 0);

			if (inputs.a.bHeld || inputs.left.bHeld)
				direction.x--;
			if (inputs.d.bHeld || inputs.right.bHeld)
				direction.x++;
			if (inputs.s.bHeld || inputs.down.bHeld)
				direction.y--;
			if (inputs.w.bHeld || inputs.up.bHeld)
				direction.y++;

			Vec2 oldPos = pos;
			if (direction != Vec2(0, 0))
				Entity::TryMove(direction, 3, *entities);

			playerVel = pos - oldPos;
		}
		playerPos = pos;

		Entity::Update(screen, entities, frameCount, inputs);
	}

	void OnDeath() override
	{
		playerAlive = false;
	}
};