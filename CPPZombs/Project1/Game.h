#include "Player.h"

class Game : public olc::PixelGameEngine
{
public:
	vector<Entity*> entities;

	float timeBetweenFrames = 0.125f;
	float currentTime = 0.0f;
	float lastTrueFrame = 0.0f;
	int frameCount = 0;

	Inputs inputs = Inputs(button(), button(), button(), button(), button(), button(), button(), button(), button(), button(), button());


	Game()
	{
		entities.push_back(new Player(Vec2(screenWidth / 2, screenHeight / 2), olc::BLUE));
		sAppName = "CPPZombs!";
	}

	virtual bool OnUserCreate()
	{
		SetPixelMode(Color::ALPHA);
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
			Update(currentTime - lastTrueFrame);
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
		Clear(olc::Pixel(olc::GREEN));

		if (inputs.leftMouse.bHeld)
			TryAndAttack(Entity::ToSpace(GetMousePos()), 1, &entities);
		else if (inputs.rightMouse.bHeld && EmptyFromEntities(Entity::ToSpace(GetMousePos()), entities))
			entities.push_back(new DToCol(GetMousePos(), olc::YELLOW, Color(0, 0, 0, 127), 1, 4, 4));

		for(int i = 0; i < entities.size(); i++)
			entities[i]->Update(this, entities, frameCount, inputs);

		if (frameCount % 6 < 4)
			Draw(GetMouseX(), GetMouseY(), Color(0, 0, 0, 127));
		frameCount++;
	}

	virtual bool OnUserDestroy()
	{
		for (int i = 0; i < entities.size(); i++)
			delete entities[i];
		return true;
	}
};