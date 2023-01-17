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

		if (playerAlive)
			Update(deltaTime);
		else
		{
			Clear(olc::BLACK);
			DrawString(Vec2(0, 0), to_string(totalGamePoints) + "\n\nKilled by:\n" + deathCauseName + "\n\nPress esc\nto close.");
		}

		inputs.Update2();
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
	if (tTime - lastWave > secondsBetweenWaves && frameCount != 0 || (inputs.enter.bPressed))
	{
		if (!inputs.enter.bPressed)
			lastWave = tTime;

		waveCount += int(!inputs.enter.bPressed);

		if (waveCount == 0)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = RandFloat() * 6.283184f;
				ranger->BetterClone(this, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if(waveCount == 7)
			for (int i = 0; i < 5; i++) // Deceiver, 5 on wave 7, First on wave 6, 1 = x/3 - 1, x/3 = 2, x = 6
			{
				float randomValue = RandFloat() * 6.283184f;
				deceiver->BetterClone(this, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if (waveCount == 9)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = RandFloat() * 6.283184f;
				parent->BetterClone(this, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if(waveCount == 10)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = RandFloat() * 6.283184f;
				hyperSpeedster->BetterClone(this, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if (waveCount == 13)
			for (int i = 0; i < 5; i++)
			{
				float randomValue = RandFloat() * 6.283184f;
				gigaExploder->BetterClone(this, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else if(waveCount == 15)
			for (int i = 0; i < 30; i++)
			{
				float randomValue = RandFloat() * 6.283184f;
				megaTanker->BetterClone(this, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
			}
		else
		{
			int totalCost = 0;
			int costToAchieve = static_cast<int>(pow(1.37, waveCount)) + waveCount * 3 - 1;
			int currentlySpawnableEnemyCount = 0;
			for (int i = 0; i < spawnableEnemyTypes.size(); i++)
				currentlySpawnableEnemyCount += int(spawnableEnemyTypes[i]->firstWave <= waveCount && spawnableEnemyTypes[i]->Cost() <= costToAchieve);
			vector<Enemy*> currentlySpawnableEnemies(currentlySpawnableEnemyCount);
			for (int i = 0, j = 0; j < currentlySpawnableEnemyCount; i++)
				if (spawnableEnemyTypes[i]->firstWave <= waveCount && spawnableEnemyTypes[i]->Cost() <= costToAchieve)
					currentlySpawnableEnemies[j++] = spawnableEnemyTypes[i];
			
			while (totalCost < costToAchieve)
			{
				int currentIndex = rand() % currentlySpawnableEnemyCount;
				float randomValue = RandFloat() * 6.283184f;
				currentlySpawnableEnemies[currentIndex]->BetterClone(this, Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos);
				totalCost += currentlySpawnableEnemies[currentIndex]->Cost();
			}
		}
	}


	entities->Update(this, dTime); // Updates all entities.

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
	entities->DUpdate(this, dTime); // Draws all entities.
	// Draw mouse.
	Draw(ToRSpace(inputs.mousePosition), Color(0, 0, 0, static_cast<uint8_t>((sinf(tTime * 3.14f * 3.0f) + 1.0f) * 64)));
	// Reset screen to high-res screen.
	SetDrawTarget(nullptr); // nullptr means default here.
	DrawSprite({ 0, 0 }, &midResScreen, 4); // Apply the mid-res screen onto the big one before use.
	entities->UIUpdate(this, dTime); // Draws all entities.
	
	if (tTime < 2.0f)
	{
		uint8_t opacity = static_cast<uint8_t>((2.0 - tTime) * 128 - 1);
		FillRect(vZero, Vec2(ScreenWidth(), ScreenHeight()), Color(0, 0, 0, opacity));
		DrawString(vZero, "OneLoneCoder.com\nPixel Game Engine",
			Color(255, 255, 255, opacity), 2);
	}



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

void Game::MenuedEntityDied(Entity* entity)
{
	if (player->currentMenuedEntity == entity)
		player->currentMenuedEntity = nullptr;
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