#include "Player.h"

bool Game::OnUserCreate()
{
	screen = olc::Sprite(screenWidth / 4, screenHeight / 4);
	entities = new Entities(0);
	player = new Player(ToSpace(screenDimH), vOne, 6, olc::BLUE, 1, 10, 5, "Player");
	entities->push_back(player);
	playerAlive = true;
	totalGamePoints = 0;
	sAppName = "CPPZombs!";
	SetPixelMode(Color::ALPHA);

	backgroundNoise.SetFrequency(0.06125f);
	backgroundNoise.SetFractalLacunarity(2.0f);
	backgroundNoise.SetFractalGain(0.5f);
	backgroundNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
	srand(static_cast<uint>(time(NULL)));
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



		inputs.mousePosition = ToSpace(GetMousePos()) + playerPos - screenDimH;

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

Color Game::GetBackgroundNoise(Color baseColor, Vec2f noisePos)
{
	return Color(min(255, max(0, baseColor.r + (int)roundf(backgroundNoise.GetNoise(noisePos.x, noisePos.y) * 5.0f) * 5)), baseColor.g, baseColor.b);
}

void Game::Update(float dTime)
{
	tTime += dTime;

	system_clock::time_point timeStartFrame = system_clock::now();

	// New wave:
	if (tTime - lastWave > secondsBetweenWaves && frameCount != 0 || inputs.enter.bPressed)
	{
		if (!inputs.enter.bPressed)
			lastWave = tTime;

		waveCount += int(!inputs.enter.bPressed);

		if(waveCount == 10)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new Enemy(hyperSpeedster, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}
		else if(waveCount == 7)
			for (int i = 0; i < 5; i++) // Deceiver, 5 on wave 7, First on wave 6, 1 = x/3 - 1, x/3 = 2, x = 6
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new EnemyClasses::Deceiver(deceiver, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}
		else if(waveCount == 15)
			for (int i = 0; i < 30; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new Enemy(megaTanker, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}
		else if (waveCount == 0 || waveCount == 9)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new EnemyClasses::Parent(parent, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}
		else
		{
			for (int i = 0; i < waveCount * 3 + 7; i++) // Walker
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new Enemy(walker, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}

			for (int i = 0; i < waveCount * 2 - 4; i++) // Tanker, First two on wave 3, 2 = 2x - 4, 2x = 6, x = 3
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new Enemy(tanker, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}

			for (int i = 0; i < waveCount - 4; i++) // Speedster, First on wave 5, 1 = x/2 - 1, x/2 = 2, x = 8
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new Enemy(speedster, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}

			for (int i = 0; i < waveCount / 5 - 1; i++) // Hyper Speedster, 5 on wave 10, First on wave 10, 1 = x/5 - 1, x/5 = 2, x = 10
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new Enemy(hyperSpeedster, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}

			for (int i = 0; i < waveCount / 3 - 1; i++) // Deceiver, 3 on wave 6, First on wave 3, 1 = x/3 - 1, x/3 = 2, x = 6
			{
				float randomValue = ((float)rand() / (float)RAND_MAX) * 6.283184f;
				entities->push_back(new EnemyClasses::Deceiver(deceiver, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			}
		}
	}


	entities->Update(this, frameCount, inputs, dTime); // Updates all entities.

	if (playerPos != lastPlayerPos)
	{
		lastPlayerPos = playerPos;
		Vec2f spacePlayerPos = (Vec2f)ToSpace(playerPos) / 4.0f;
		Color* screenColors = screen.GetData(); // Background draw must be after player gets updated.
		for (int x = 0; x < screen.width; x++)
			for (int y = 0; y < screen.height; y++)
				screenColors[y * screen.width + x] = GetBackgroundNoise(Color(150, 92, 20), spacePlayerPos + Vec2(x, y));
	}
	DrawSprite({ 0, 0 }, &screen, 4);
		
	entities->DUpdate(this, frameCount, inputs, dTime); // Draws all entities.
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

	if ((int)(tTime * 5) % 5 < 3)
		Draw(ToRSpace(inputs.mousePosition), Color(0, 0, 0, 127));
		
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