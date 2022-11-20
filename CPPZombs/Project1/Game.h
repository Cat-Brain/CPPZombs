#include "Player.h"

#define ticsBetweenWaves 2048



class Game : public Screen
{
public:
	Entities entities;
	Player* player;
	FastNoiseLite backgroundNoise1, backgroundNoise2, backgroundNoise3;

	float timeBetweenFrames = 0.03125f;
	float currentTime = 0.0f;
	float lastTrueFrame = 0.0f;
	int frameCount = 0, waveCount = 0;
	bool showUI = true, paused = false;

	

	Game() { }

	virtual bool OnUserCreate()
	{
		screen = olc::Sprite(screenWidth, screenHeight);
		bigScreen = olc::Sprite(screenWidth * GRID_SIZE, screenHeight * GRID_SIZE);
		entities = Entities(0);
		player = new Player(ToSpace(Vec2(screenWidth / 2, screenHeight / 2)), olc::BLUE, 1, 10, 5);
		entities.push_back(player);
		playerAlive = true;
		totalGamePoints = 0;
		sAppName = "CPPZombs!";
		SetPixelMode(Color::ALPHA);

		backgroundNoise1.SetFractalLacunarity(2.0f);
		backgroundNoise1.SetFractalGain(0.5f);
		backgroundNoise1.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
		backgroundNoise2 = backgroundNoise1;
		backgroundNoise3 = backgroundNoise1;
		srand(time(NULL));
		backgroundNoise1.SetSeed(time(NULL));
		backgroundNoise2.SetSeed(time(NULL) + 1);
		backgroundNoise3.SetSeed(time(NULL) + 2);

		return true;
	}
	
	virtual bool OnUserUpdate(float deltaTime)
	{
		if (GetKey(olc::ESCAPE).bPressed)
			bAtomActive = false;

		if (GetKey(olc::P).bPressed)
			paused = !paused;

		if (!paused)
		{
			#pragma region Inputs 1

			inputs.mouseScroll += GetMouseWheel() / 120;

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

			temp = GetKey(olc::SPACE);
			inputs.space.bHeld |= temp.bHeld;
			inputs.space.bPressed |= temp.bPressed;
			inputs.space.bReleased |= temp.bReleased;

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
				inputs.mousePosition = ToSpace(GetMousePos() / 3) + playerPos - Vec2(screenWidth * 0.5f, screenHeight * 0.5f);

				if (playerAlive)
					Update(currentTime - lastTrueFrame);
				else
				{
					Clear(olc::BLACK);
					DrawString(Vec2(0, 0), to_string(totalGamePoints) + "\n\nPress esc\nto close.");
				}
				lastTrueFrame += timeBetweenFrames;

				#pragma region Inputs2

				inputs.mouseScroll = 0;

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

				inputs.space.bHeld = false;
				inputs.space.bPressed = false;
				inputs.space.bReleased = false;

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
		}

		return true;
	}

	Color GetBackgroundNoise(Color baseColor, int x, int y)
	{
		Vec2 noisePos = Vec2(x + ToSpace(playerPos).x, y + ToSpace(playerPos).y);
		return Color((int)fminf(255, fmaxf(0, baseColor.r + (int)roundf(backgroundNoise1.GetNoise((float)noisePos.x, (float)noisePos.y) * 5.0f) * 5)),
			(int)fminf(255, fmaxf(0, baseColor.g + (int)roundf(backgroundNoise2.GetNoise((float)noisePos.x, (float)noisePos.y) * 5.0f) * 3)),
			(int)fminf(255, fmaxf(0, baseColor.b + (int)roundf(backgroundNoise3.GetNoise((float)noisePos.x, (float)noisePos.y) * 5.0f)) * 2));
	}

	void Update(float deltaTime)
	{
		if (frameCount % ticsBetweenWaves == 0 && frameCount != 0 || inputs.enter.bPressed)
		{
			waveCount += int(!inputs.enter.bPressed);
			for (int i = 0; i < waveCount * 3 + 7; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities.push_back(new Enemy(walker, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.412f + playerPos));
			}
		}


		entities.Update(this, frameCount, inputs); // Updates all entities.

		if (playerPos != lastPlayerPos)
		{
			lastPlayerPos = playerPos;
			Color* screenColors = screen.GetData(); // Background draw must be after player gets updated.
			for (int x = 0; x < screen.width; x++)
				for (int y = 0; y < screen.height; y++)
					screenColors[y * screen.width + x] = GetBackgroundNoise(Color(150, 92, 20), x, y);
		}
		DrawScreen();
		
		entities.DUpdate(this, frameCount, inputs); // Draws all entities.



		if (inputs.c.bPressed)
			showUI = !showUI;
		if (showUI && playerAlive)
		{
			DrawString(Vec2(0, 0), std::to_string(ticsBetweenWaves - frameCount % ticsBetweenWaves) + " - " + std::to_string(waveCount), olc::BLACK);
			DrawString(Vec2(0, 9), std::to_string(entities[0]->health), olc::DARK_RED);
			DrawString(Vec2(0, 18), to_string(totalGamePoints), olc::DARK_YELLOW);
			player->items.DUpdate(this);
		}

		if (frameCount % 6 < 4)
			Draw(ToRSpace(inputs.mousePosition) + Vec2(1, 1), Color(0, 0, 0, 127));



		frameCount++;
	}

	virtual bool OnUserDestroy()
	{
		for (int i = 0; i < entities.size(); i++)
			delete entities[i];
		return true;
	}
};