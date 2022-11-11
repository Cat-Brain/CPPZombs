#include "Player.h"

#define ticsBetweenWaves 512



class Game : public olc::PixelGameEngine
{
public:
	Entities entities;

	float timeBetweenFrames = 0.125f;
	float currentTime = 0.0f;
	float lastTrueFrame = 0.0f;
	int frameCount = 0, waveCount = 0;
	bool showUI = true;

	Inputs inputs = Inputs(button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), Vec2(0, 0));


	Game()
	{
		entities = Entities(0);
		entities.push_back(new Player(Entity::ToSpace(Vec2(screenWidth / 2, screenHeight / 2)), olc::BLUE, 1, 10, 5));
		playerAlive = true;
		totalGamePoints = 0;
		sAppName = "CPPZombs!";
	}

	virtual bool OnUserCreate()
	{
		SetPixelMode(Color::ALPHA);
		srand(time(NULL));
		return true;
	}

	virtual bool OnUserUpdate(float deltaTime)
	{
		if (GetKey(olc::ESCAPE).bPressed)
			bAtomActive = false;

		#pragma region Inputs 1

		button temp = GetKey(olc::W);
		inputs.w.bHeld |= temp.bHeld;
		inputs.w.bPressed |= temp.bPressed;
		inputs.w.bReleased |= temp.bReleased;

		temp = GetKey(olc::A);
		inputs.a.bHeld |= temp.bHeld;
		inputs.a.bPressed |= temp.bPressed;
		inputs.a.bReleased |= temp.bReleased;

		temp = GetKey(olc::S);
		inputs.s.bHeld |= temp.bHeld;
		inputs.s.bPressed |= temp.bPressed;
		inputs.s.bReleased |= temp.bReleased;

		temp = GetKey(olc::D);
		inputs.d.bHeld |= temp.bHeld;
		inputs.d.bPressed |= temp.bPressed;
		inputs.d.bReleased |= temp.bReleased;

		temp = GetKey(olc::ENTER);
		inputs.enter.bHeld |= temp.bHeld;
		inputs.enter.bPressed |= temp.bPressed;
		inputs.enter.bReleased |= temp.bReleased;

		temp = GetKey(olc::C);
		inputs.c.bHeld |= temp.bHeld;
		inputs.c.bPressed |= temp.bPressed;
		inputs.c.bReleased |= temp.bReleased;

		temp = GetKey(olc::Q);
		inputs.q.bHeld |= temp.bHeld;
		inputs.q.bPressed |= temp.bPressed;
		inputs.q.bReleased |= temp.bReleased;

		temp = GetKey(olc::E);
		inputs.e.bHeld |= temp.bHeld;
		inputs.e.bPressed |= temp.bPressed;
		inputs.e.bReleased |= temp.bReleased;

		temp = GetKey(olc::UP);
		inputs.up.bHeld |= temp.bHeld;
		inputs.up.bPressed |= temp.bPressed;
		inputs.up.bReleased |= temp.bReleased;

		temp = GetKey(olc::LEFT);
		inputs.left.bHeld |= temp.bHeld;
		inputs.left.bPressed |= temp.bPressed;
		inputs.left.bReleased |= temp.bReleased;

		temp = GetKey(olc::DOWN);
		inputs.down.bHeld |= temp.bHeld;
		inputs.down.bPressed |= temp.bPressed;
		inputs.down.bReleased |= temp.bReleased;

		temp = GetKey(olc::RIGHT);
		inputs.right.bHeld |= temp.bHeld;
		inputs.right.bPressed |= temp.bPressed;
		inputs.right.bReleased |= temp.bReleased;

		temp = GetMouse(0);
		inputs.leftMouse.bHeld |= temp.bHeld;
		inputs.leftMouse.bPressed |= temp.bPressed;
		inputs.leftMouse.bReleased |= temp.bReleased;

		temp = GetMouse(1);
		inputs.rightMouse.bHeld |= temp.bHeld;
		inputs.rightMouse.bPressed |= temp.bPressed;
		inputs.rightMouse.bReleased |= temp.bReleased;

		temp = GetMouse(2);
		inputs.middleMouse.bHeld |= temp.bHeld;
		inputs.middleMouse.bPressed |= temp.bPressed;
		inputs.middleMouse.bReleased |= temp.bReleased;

		#pragma endregion


		currentTime += deltaTime;
		if (currentTime > lastTrueFrame + timeBetweenFrames)
		{
			inputs.mousePosition = Entity::ToSpace(GetMousePos() / 3) + playerPos - Vec2(screenWidth * 0.5f, screenHeight * 0.5f);

			if (playerAlive)
				Update(currentTime - lastTrueFrame);
			else
			{
				Clear(olc::BLACK);
				DrawString(Vec2(0, 0), to_string(totalGamePoints) + "\n\nPress esc\nto close.");
			}
			lastTrueFrame += timeBetweenFrames;

			#pragma region Inputs2

			inputs.w.bHeld = false;
			inputs.w.bPressed = false;
			inputs.w.bReleased = false;

			inputs.a.bHeld = false;
			inputs.a.bPressed = false;
			inputs.a.bReleased = false;

			inputs.s.bHeld = false;
			inputs.s.bPressed = false;
			inputs.s.bReleased = false;

			inputs.d.bHeld = false;
			inputs.d.bPressed = false;
			inputs.d.bReleased = false;

			inputs.enter.bHeld = false;
			inputs.enter.bPressed = false;
			inputs.enter.bReleased = false;

			inputs.c.bHeld = false;
			inputs.c.bPressed = false;
			inputs.c.bReleased = false;

			inputs.q.bHeld = false;
			inputs.q.bPressed = false;
			inputs.q.bReleased = false;

			inputs.e.bHeld = false;
			inputs.e.bPressed = false;
			inputs.e.bReleased = false;

			inputs.up.bHeld = false;
			inputs.up.bPressed = false;
			inputs.up.bReleased = false;

			inputs.left.bHeld = false;
			inputs.left.bPressed = false;
			inputs.left.bReleased = false;

			inputs.down.bHeld = false;
			inputs.down.bPressed = false;
			inputs.down.bReleased = false;

			inputs.right.bHeld = false;
			inputs.right.bPressed = false;
			inputs.right.bReleased = false;

			inputs.leftMouse.bHeld = false;
			inputs.leftMouse.bPressed = false;
			inputs.leftMouse.bReleased = false;

			inputs.rightMouse.bHeld = false;
			inputs.rightMouse.bPressed = false;
			inputs.rightMouse.bReleased = false;

			inputs.middleMouse.bHeld = false;
			inputs.middleMouse.bPressed = false;
			inputs.middleMouse.bReleased = false;

			#pragma endregion
		}

		return true;
	}

	void Update(float deltaTime)
	{
		Clear(olc::Pixel(200, 92, 20));


		if (frameCount % ticsBetweenWaves == 0 && frameCount != 0 || inputs.enter.bPressed)
		{
			waveCount++;
			for (int i = 0; i < waveCount * 3; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities.push_back(new Walker(Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.142f + playerPos, olc::CYAN, olc::BLACK, 1, 3, 3));
			}
		}


		if (inputs.leftMouse.bPressed && inputs.mousePosition != playerPos)
			entities.push_back(new Projectile(entities[0]->pos, inputs.mousePosition, 10, 0, olc::GREY, 1, 1, 1));
			//TryAndAttack(Entity::ToSpace(GetMousePos()), 1, &entities);
		if (inputs.rightMouse.bHeld && EmptyFromEntities(inputs.mousePosition, entities) && playerAlive)
			entities.push_back(new Placeable(inputs.mousePosition, olc::YELLOW, Color(0, 0, 0, 127), 1, 4, 4));




		entities.Update(this, frameCount, inputs); // Updates all entities.

		if (inputs.c.bPressed)
			showUI = !showUI;
		if (showUI)
		{
			DrawString(Vec2(0, 0), std::to_string(ticsBetweenWaves - frameCount % ticsBetweenWaves) + " - " + std::to_string(waveCount), olc::BLACK);
			DrawString(Vec2(0, 9), std::to_string(entities[0]->health), olc::DARK_RED);
			DrawString(Vec2(0, 18), to_string(totalGamePoints), olc::DARK_YELLOW);
		}

		if (frameCount % 6 < 4)
			Draw(Entity::ToRSpace(inputs.mousePosition) + Vec2(1, 1), Color(0, 0, 0, 127));



		frameCount++;
	}

	virtual bool OnUserDestroy()
	{
		for (int i = 0; i < entities.size(); i++)
			delete entities[i];
		return true;
	}
};