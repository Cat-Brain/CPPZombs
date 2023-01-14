#include "Player.h"

bool Game::OnUserCreate()
{
	lowResScreen = olc::Sprite(screenWidth / 4, screenHeight / 4);
	midResScreen = olc::Sprite(screenWidth, screenHeight);
	SetDrawTarget(&midResScreen);
	entities = new Entities(0);
	player = new Player(ToSpace(screenDimH), vOne, 6, olc::BLUE, 1, 10, 5, "Player");
	entities->push_back(player);
	playerAlive = true;
	totalGamePoints = 0;
	sAppName = "CPPZombs!";
	SetPixelMode(Color::ALPHA);

	srand(static_cast<uint>(time(NULL)));
	backgroundBaseColor.r = rand() % 128 + 64;
	backgroundBaseColor.g = rand() % 128 + 64;
	backgroundBaseColor.b = rand() % 128 + 64;

	int randChoice = rand();
	backgroundColorWidth.r = rand() % 2 + 2 + 4 * int(randChoice == 0);
	backgroundColorWidth.g = rand() % 2 + 2 + 4 * int(randChoice == 1);
	backgroundColorWidth.b = rand() % 2 + 2 + 4 * int(randChoice == 2);

	backgroundNoise.SetFrequency(0.06125f);
	backgroundNoise.SetFractalLacunarity(2.0f);
	backgroundNoise.SetFractalGain(0.5f);
	backgroundNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
	backgroundNoise.SetSeed(static_cast<int>(time(NULL)));

	return true;
}
	
bool Game::OnUserUpdate(float deltaTime)
{
	if (GetKey(olc::ESCAPE).bPressed)
		bAtomActive = false;

	if (GetKey(olc::P).bPressed && !IsConsoleShowing())
		paused = !paused;
	if (GetKey(olc::F1).bPressed)
	{
		paused = true;
		ConsoleShow(olc::F1, false);
	}

	if (!paused)
	{
		inputs.Update1(this);

		inputs.mousePosition = ToSpace(GetMousePos() / 4) + playerPos - screenDimH;

		if (playerAlive)
			Update(deltaTime);
		else
		{
			Clear(olc::BLACK);
			DrawString(Vec2(0, 0), to_string(totalGamePoints) + "\n\nKilled by:\n" + deathCauseName + "\n\nPress esc\nto close.");
		}

		#pragma region Inputs2

		inputs.mouseScroll = 0;
		// Not held wasd or arrow keys as that's covered by the player.
		inputs.a.bPressed = false;
		inputs.a.bReleased = false;

		inputs.left.bPressed = false;
		inputs.left.bReleased = false;

		inputs.d.bPressed = false;
		inputs.d.bReleased = false;

		inputs.right.bPressed = false;
		inputs.right.bReleased = false;

		inputs.s.bPressed = false;
		inputs.s.bReleased = false;

		inputs.down.bPressed = false;
		inputs.down.bReleased = false;

		inputs.w.bPressed = false;
		inputs.w.bReleased = false;

		inputs.up.bPressed = false;
		inputs.up.bReleased = false;

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

Color Game::GetBackgroundNoise(Vec2f noisePos)
{
	float randomNoiseValue = backgroundNoise.GetNoise(noisePos.x, noisePos.y);
	return Color(Clamp(backgroundBaseColor.r + (int)roundf(randomNoiseValue * backgroundColorWidth.r) * 5, 0, 255),
		Clamp(backgroundBaseColor.g + (int)roundf(randomNoiseValue * backgroundColorWidth.g) * 5, 0, 255),
		Clamp(backgroundBaseColor.b + (int)roundf(randomNoiseValue * backgroundColorWidth.b) * 5, 0, 255));
}

void Game::Update(float dTime)
{
	tTime += dTime;

	system_clock::time_point timeStartFrame = system_clock::now();

	// New wave:
	if (tTime - lastWave > secondsBetweenWaves && frameCount != 0 || (inputs.enter.bPressed && waveCount != 13))
	{
		if (!inputs.enter.bPressed)
			lastWave = tTime;

		waveCount += int(!inputs.enter.bPressed);

		if(waveCount == 10)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				hyperSpeedster->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if(waveCount == 7)
			for (int i = 0; i < 5; i++) // Deceiver, 5 on wave 7, First on wave 6, 1 = x/3 - 1, x/3 = 2, x = 6
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				deceiver->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if(waveCount == 15)
			for (int i = 0; i < 30; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				megaTanker->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if (waveCount == 9)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				parent->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if (waveCount == 13)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				gigaExploder->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if (waveCount == 0)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				snake->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else
		{
			for (int i = 0; i < waveCount * 3 + 7; i++) // Walker
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				walker->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}

			for (int i = 0; i < waveCount * 2 - 4; i++) // Tanker, First two on wave 3, 2 = 2x - 4, 2x = 6, x = 3
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				tanker->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}

			for (int i = 0; i < waveCount - 4; i++) // Speedster, First on wave 5, 1 = x/2 - 1, x/2 = 2, x = 8
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				speedster->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}

			for (int i = 0; i < waveCount / 5 - 1; i++) // Hyper Speedster, 5 on wave 10, First on wave 10, 1 = x/5 - 1, x/5 = 2, x = 10
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				hyperSpeedster->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}

			for (int i = 0; i < waveCount / 3 - 1; i++) // Deceiver, 3 on wave 6, First on wave 3, 1 = x/3 - 1, x/3 = 2, x = 6
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				deceiver->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}

			for (int i = 0; i < waveCount / 3 - 2; i++) // Parent, 5 on wave 9, First on wave 9
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				parent->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}

			for (int i = 0; i < waveCount / 2 - 2; i++) // Exploder, First on wave 6
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				exploder->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}

			for (int i = 0; i < waveCount / 2 - 3; i++) // Exploder, First on wave 8
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				snake->BetterClone(this, entities, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		}
	}


	entities->Update(this, frameCount, inputs, dTime); // Updates all entities.

	if (playerPos != lastPlayerPos)
	{
		lastPlayerPos = playerPos;
		Vec2f spacePlayerPos = (Vec2f)ToSpace(playerPos) / 4.0f;
		Color* screenColors = lowResScreen.GetData(); // Background draw must be after player gets updated.
		for (int x = 0; x < lowResScreen.width; x++)
			for (int y = 0; y < lowResScreen.height; y++)
				screenColors[y * lowResScreen.width + x] = GetBackgroundNoise(spacePlayerPos + Vec2(x, y));
	}
	SetDrawTarget(&midResScreen);
	DrawSprite({ 0, 0 }, &lowResScreen, 4);
	entities->DUpdate(this, frameCount, inputs, dTime); // Draws all entities.
	// Draw mouse.
	Draw(ToRSpace(inputs.mousePosition), Color(0, 0, 0, (sinf(tTime * 3.14f * 3.0f) + 1.0f) * 64));
	// Reset screen to high-res screen.
	SetDrawTarget(nullptr); // nullptr means default here.
	DrawSprite({ 0, 0 }, &midResScreen, 4); // Apply the mid-res screen onto the big one before use.
	entities->UIUpdate(this, frameCount, inputs, dTime); // Draws all entities.



	if (inputs.c.bPressed)
		showUI = !showUI;
	if (showUI && playerAlive)
	{
		DrawString(Vec2(0, 0), ToStringWithPrecision((secondsBetweenWaves - tTime + lastWave), 1) + " - " + std::to_string(waveCount), olc::BLACK);
		DrawString(Vec2(0, 9), std::to_string(player->health), olc::DARK_RED);
		DrawString(Vec2(0, 18), to_string(totalGamePoints), olc::DARK_YELLOW);
		player->items.DUpdate(this);
	}

	frameCount++;
}

bool Game::OnUserDestroy()
{
	for (int i = 0; i < entities->size(); i++)
		delete (*entities)[i];
	return true;
}

bool Game::OnConsoleCommand(const string& text)
{
	std::stringstream stream;
	stream << text;

	string firstWord;
	if (stream >> firstWord)
	{
		if (firstWord == "SetWave")
		{
			string secondWord;
			if (stream >> secondWord)
			{
				int newWaveCount = stoi(secondWord);
				if (newWaveCount > 0)
					waveCount = newWaveCount;
			}
		}
		else if (firstWord == "SetPlayerSize")
		{
			string secondWord;
			if (stream >> secondWord)
			{
				int newWidth = stoi(secondWord);
				string thirdWord;
				if (newWidth != -1 && stream >> thirdWord)
				{
					int newHeight = stoi(thirdWord);
					if (newHeight != -1)
						player->dimensions = Vec2(newWidth, newHeight);
				}
			}
		}
	}
	return true;
}