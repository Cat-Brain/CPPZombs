#include "Planet.h"

void Game::Start()
{
	srand(static_cast<uint>(time(NULL)));

	entities = make_unique<Entities>();
	unique_ptr<Player> playerUnique = make_unique<Player>(vOne * (CHUNK_WIDTH * MAP_WIDTH) / 2, vOne, 6, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), RGBA(), 25, 1, 10, 5, "Player");
	player = static_cast<Player*>(playerUnique.get());
	entities->push_back(std::move(playerUnique));
	playerAlive = true;
	totalGamePoints = 0;

	planet = std::make_unique<Planet>();
}
	
void Game::Update()
{
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		paused = !paused;
	/*if (GetKey(olc::F1).bPressed)
	{
		paused = true;
		ConsoleShow(olc::F1, false);
	}*/

	if (!paused)
	{
		if (playerAlive)
			TUpdate();
		else
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			//DrawString(Vec2(0, 0), to_string(totalGamePoints) + "\n\nKilled by:\n" + deathCauseName + "\n\nPress esc\nto close.");
			DrawFramebufferOnto(0);
		}
	}
}

void Game::ApplyLighting()
{
	JRGB ambientColor = JRGB(static_cast<byte>(brightness * 255), static_cast<byte>(brightness * 255), static_cast<byte>(brightness * 255));

	/*Color* renderData = GetDrawTarget()->GetData();

	for (unique_ptr<LightSource>& lightSource : entities->lightSources)
		lightSource->ApplyLight();

	for (int x = screenWidth; x < screenWidth * 2; x++)
		for (int y = screenHeight; y < screenHeight * 2; y++)
		{
			int index = x - screenWidth + (screenHeight * 2 - y - 1) * screenWidth;
			renderData[index].r = static_cast<byte>((int)renderData[index].r * shadowMap[x][y].r / 255);
			renderData[index].g = static_cast<byte>((int)renderData[index].g * shadowMap[x][y].g / 255);
			renderData[index].b = static_cast<byte>((int)renderData[index].b * shadowMap[x][y].b / 255);
			shadowMap[x][y] = ambientColor;
		}*/
}

void Game::TUpdate()
{
	inputs.FindMousePos(window);

	brightness = planet->GetBrightness();

	system_clock::time_point timeStartFrame = system_clock::now();

	// Spawn boss:
	if (inputs.enter.pressed && !shouldSpawnBoss)
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
	if (tTime - lastWave > secondsBetweenWaves && frameCount != 0 || inputs.enter.pressed)
	{
		lastWave = tTime;
		waveCount++;
		planet->enemies.SpawnRandomEnemies();
	}

	glUseProgram(backgroundShader);
	glUniform2f(glGetUniformLocation(backgroundShader, "offset"), PlayerPos().x * 2, PlayerPos().y * 2);
	glUniform2f(glGetUniformLocation(backgroundShader, "screenDim"), ScrWidth(), ScrHeight());
	screenSpaceQuad.Draw();

	entities->Update(); // Updates all entities.
	entities->DUpdate(); // Draws all entities.
	ApplyLighting(); // Apply lighting.
	Draw(inputs.mousePosition + player->pos, RGBA(0, 0, 0, static_cast<uint8_t>((sinf(tTime * 3.14f * 3.0f) + 1.0f) * 64)));// Draw mouse.
	// Draw mid-res screen onto true screen.
	DrawFramebufferOnto(0);

	entities->UIUpdate(); // Draws UI of uiactive entities.
	


	if (inputs.c.pressed)
		showUI = !showUI;
	if (showUI && playerAlive)
	{
		float timeTillNextWave = secondsBetweenWaves - tTime + lastWave;
		if (shouldSpawnBoss)
		{
			float timeTillNextBoss = 60.0f - tTime + timeStartBossPrep;
			//DrawString(Vec2(0, 0), std::to_string(int(timeTillNextWave)) + "." +
			//	std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount) + " " +
			//	std::to_string(int(timeTillNextBoss)) + "." +
			//	std::to_string(int(timeTillNextBoss * 10) - int(timeTillNextBoss) * 10), olc::CYAN);
		}
		//else
			//DrawString(Vec2(0, 0), std::to_string(int(timeTillNextWave)) + "." +
			//	std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount), olc::CYAN);
		//DrawString(Vec2(0, 9), std::to_string(player->health), olc::DARK_RED);
		//DrawString(Vec2(0, 18), to_string(totalGamePoints), olc::DARK_YELLOW);
		player->items.DUpdate();
	}

	frameCount++;
}

void Game::MenuedEntityDied(Entity* entity)
{
	if (player->currentMenuedEntity == entity)
		player->currentMenuedEntity = nullptr;
}

void Game::End()
{
	// Maybe needed later.
}