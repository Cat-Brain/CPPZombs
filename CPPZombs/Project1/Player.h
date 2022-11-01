#include "Projectile.h"

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

			if (direction != Vec2(0, 0))
				Entity::TryMove(direction, 3, *entities);
		}

		Entity::Update(screen, entities, frameCount, inputs);
	}

	bool CanAttack() override
	{
		return false;
	}
};