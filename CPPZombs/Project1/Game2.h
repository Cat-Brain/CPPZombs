#include "Planet.h"

bool Game::OnUserCreate()
{
	srand(static_cast<uint>(time(NULL)));

	lowResScreen = olc::Sprite(screenWidth / 4, screenHeight / 4);
	midResScreen = olc::Sprite(screenWidth, screenHeight);
	SetDrawTarget(&midResScreen);
	entities = make_unique<Entities>();
	player = make_shared<Player>(vOne * (CHUNK_WIDTH * MAP_WIDTH) / 2, vOne, 6, olc::BLUE, olc::BLACK, JRGB(127, 127, 127), olc::BLACK, 25, 1, 10, 5, "Player");
	entities->push_back(player);
	playerAlive = true;
	totalGamePoints = 0;
	sAppName = "CPPZombs!";
	SetPixelMode(Color::ALPHA);

	planet = std::make_unique<Planet>();

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

		dTime = deltaTime;

		if (playerAlive)
			Update();
		else
		{
			Clear(olc::BLACK);
			DrawString(Vec2(0, 0), to_string(totalGamePoints) + "\n\nKilled by:\n" + deathCauseName + "\n\nPress esc\nto close.");
		}

		inputs.Update2();
	}

	return true;
}

void Game::ApplyLighting()
{
	JRGB ambientColor = JRGB(static_cast<byte>(brightness * 255), static_cast<byte>(brightness * 255), static_cast<byte>(brightness * 255));

	Color* renderData = GetDrawTarget()->GetData();

	for (shared_ptr<LightSource> lightSource : entities->lightSources)
		lightSource->ApplyLight();

	for (int x = screenWidth; x < screenWidth * 2; x++)
		for (int y = screenHeight; y < screenHeight * 2; y++)
		{
			int index = x - screenWidth + (screenHeight * 2 - y - 1) * screenWidth;
			renderData[index].r = static_cast<byte>((int)renderData[index].r * shadowMap[x][y].r / 255);
			renderData[index].g = static_cast<byte>((int)renderData[index].g * shadowMap[x][y].g / 255);
			renderData[index].b = static_cast<byte>((int)renderData[index].b * shadowMap[x][y].b / 255);
			shadowMap[x][y] = ambientColor;
		}
}

void Game::Update()
{
	tTime += dTime;
	brightness = planet->GetBrightness();

	system_clock::time_point timeStartFrame = system_clock::now();

	// Spawn boss:
	if (inputs.enter.bPressed && !shouldSpawnBoss)
	{
		shouldSpawnBoss = true;
		timeStartBossPrep = tTime;
	}

	if (shouldSpawnBoss && tTime - timeStartBossPrep >= 60.0f)
	{
		planet->bosses.SpawnRandomEnemies(max(250, Enemies::GetRoundPoints()));
		shouldSpawnBoss = false;
	}

	// New wave:
	if (tTime - lastWave > secondsBetweenWaves && frameCount != 0)
	{
		lastWave = tTime;
		waveCount++;
		planet->enemies.SpawnRandomEnemies();
	}

	SetDrawTarget(&midResScreen);

	entities->Update(); // Updates all entities.

	if (playerPos != lastPlayerPos)
	{
		lastPlayerPos = playerPos;
		Vec2f spacePlayerPos = (Vec2f)ToSpace(playerPos) / 4.0f;
		Color* screenColors = lowResScreen.GetData(); // Background draw must be after player gets updated.
		for (int x = 0; x < lowResScreen.width; x++)
			for (int y = 0; y < lowResScreen.height; y++)
				screenColors[y * lowResScreen.width + x] = planet->GetBackgroundNoise(spacePlayerPos + Vec2(x, y));
	}
	DrawSprite({ 0, 0 }, &lowResScreen, 4);
	entities->DUpdate(); // Draws all entities.
	// Draw mouse.
	ApplyLighting(); // Apply lighting.
	Draw(ToRSpace(inputs.mousePosition), Color(0, 0, 0, static_cast<uint8_t>((sinf(tTime * 3.14f * 3.0f) + 1.0f) * 64)));
	// Reset screen to high-res screen.
	SetDrawTarget(nullptr); // nullptr means default here.
	DrawSprite({ 0, 0 }, &midResScreen, 4); // Apply the mid-res screen onto the big one before use.
	entities->UIUpdate(); // Draws all entities.
	
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
		float timeTillNextWave = secondsBetweenWaves - tTime + lastWave;
		if (shouldSpawnBoss)
		{
			float timeTillNextBoss = 60.0f - tTime + timeStartBossPrep;
			DrawString(Vec2(0, 0), std::to_string(int(timeTillNextWave)) + "." +
				std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount) + " " +
				std::to_string(int(timeTillNextBoss)) + "." +
				std::to_string(int(timeTillNextBoss * 10) - int(timeTillNextBoss) * 10), olc::CYAN);
		}
		else
			DrawString(Vec2(0, 0), std::to_string(int(timeTillNextWave)) + "." +
				std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount), olc::CYAN);
		DrawString(Vec2(0, 9), std::to_string(player->health), olc::DARK_RED);
		DrawString(Vec2(0, 18), to_string(totalGamePoints), olc::DARK_YELLOW);
		player->items.DUpdate();
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