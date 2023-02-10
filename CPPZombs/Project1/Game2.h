#include "Planet.h"

void Game::Start()
{
	srand(static_cast<uint>(time(NULL)));

	entities = make_unique<Entities>();
	unique_ptr<Player> playerUnique = make_unique<Player>(vOne * (CHUNK_WIDTH * MAP_WIDTH) / 2, vOne, 6, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), RGBA(0, 0, 127), 25, 1, 10, 5, "Player");
	player = static_cast<Player*>(playerUnique.get());
	entities->push_back(std::move(playerUnique));
	playerAlive = true;
	totalGamePoints = 0;

	planet = std::make_unique<Planet>();
}
	
void Game::Update()
{
	if (inputs.p.pressed)
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
			currentFramebuffer = 0;
			UseFramebuffer();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			font.Render(to_string(totalGamePoints) + " points", {-ScrWidth(), int(ScrHeight() * 0.9f)}, ScrHeight() / 10.0f, { 255, 255, 255 });
			font.Render("Killed by : ", { -ScrWidth(), int(ScrHeight() * 0.7f)  }, ScrHeight() / 10.0f, { 255, 255, 255 });
			font.Render(deathCauseName, { -ScrWidth(), int(ScrHeight() * 0.6f) }, ScrHeight() / 10.0f, { 255, 255, 255 });
			font.Render("Press esc to close.", { -ScrWidth(), int(ScrHeight() * 0.4f) }, ScrHeight() / 10.0f, { 255, 255, 255 });
		}
	}
}

void Game::ApplyLighting()
{
	JRGB ambientColor = planet->GetAmbient(brightness);

	currentFramebuffer = 2;
	UseFramebuffer();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	entities->SubScatUpdate();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	currentFramebuffer = 3;
	UseFramebuffer();

	glUseProgram(shadowShader);
	glClearColor(ambientColor.r / 255.0f, ambientColor.g / 255.0f, ambientColor.b / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform2i(glGetUniformLocation(shadowShader, "scrDim"), ScrWidth(), ScrHeight());
	for (int i = 0; i < entities->lightSources.size(); i++)
	{
		LightSource* light = entities->lightSources[i].get();

		glUniform2i(glGetUniformLocation(shadowShader, "center"), light->pos.x, light->pos.y);
		glUniform1f(glGetUniformLocation(shadowShader, "range"), light->range);

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform3f(glGetUniformLocation(shadowShader, "color"), light->color.r / 255.0f, light->color.g / 255.0f, light->color.b / 255.0f);
		quad.Draw();
	}

	glUseProgram(shadingShader);
	glBindTexture(GL_TEXTURE_2D, framebuffers[currentFramebuffer - 1]->textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
	glUniform1i(glGetUniformLocation(shadingShader, "shadowTexture"), currentFramebuffer - 1);
	glUniform2i(glGetUniformLocation(shadingShader, "scrDim"), ScrWidth(), ScrHeight());

	currentFramebuffer = 1;
	UseFramebuffer();

	glUniform1i(glGetUniformLocation(shadingShader, "screenTexture"), currentFramebuffer - 1);

	screenSpaceQuad.Draw();
	glUseProgram(defaultShader);
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
		planet->bosses.SpawnRandomEnemies();
		shouldSpawnBoss = false;
	}
	waveCount += int(inputs.e.pressed) - int(inputs.q.pressed);
	// New wave:
	if (tTime - lastWave > secondsBetweenWaves && frameCount != 0 || inputs.c.pressed)
	{
		planet->enemies.SpawnRandomEnemies();
		lastWave = tTime;
		waveCount++;
	}

	glUseProgram(backgroundShader);
	glUniform2f(glGetUniformLocation(backgroundShader, "offset"), PlayerPos().x * 2, PlayerPos().y * 2);
	glUniform2f(glGetUniformLocation(backgroundShader, "screenDim"), ScrWidth(), ScrHeight());
	glUniform3f(glGetUniformLocation(backgroundShader, "col1"), planet->color1.r / 255.0f, planet->color1.g / 255.0f, planet->color1.b / 255.0f);
	glUniform3f(glGetUniformLocation(backgroundShader, "col2"), planet->color2.r / 255.0f, planet->color2.g / 255.0f, planet->color2.b / 255.0f);
	screenSpaceQuad.Draw();

	entities->Update(); // Updates all entities.
	entities->DUpdate(); // Draws all entities.
	ApplyLighting(); // Apply lighting.
	Draw(inputs.mousePosition + player->pos, RGBA(0, 0, 0, static_cast<uint8_t>((sinf(tTime * 3.14f * 3.0f) + 1.0f) * 64)));// Draw mouse.
	// Draw mid-res screen onto true screen.
	//DrawFramebufferOnto(0);

	entities->UIUpdate(); // Draws UI of uiactive entities.
	


	if (inputs.c.pressed)
		showUI = !showUI;
	if (showUI && playerAlive)
	{
		float timeTillNextWave = secondsBetweenWaves - tTime + lastWave;
		if (shouldSpawnBoss)
		{
			float timeTillNextBoss = 60.0f - tTime + timeStartBossPrep;
			font.Render(std::to_string(int(timeTillNextWave)) + "." +
				std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount) + " " +
				std::to_string(int(timeTillNextBoss)) + "." +
				std::to_string(int(timeTillNextBoss * 10) - int(timeTillNextBoss) * 10), Vec2(-ScrWidth(), ScrHeight() * 0.95f), ScrHeight() / 20.0f, RGBA(0, 255, 255));
		}
		else
			font.Render(std::to_string(int(timeTillNextWave)) + "." + std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount),
				Vec2(-ScrWidth(), ScrHeight() * 0.95f), ScrHeight() / 20.0f, RGBA(0, 255, 255));
		font.Render(std::to_string(player->health), Vec2(-ScrWidth(), ScrHeight() * 0.9f), ScrHeight() / 20.0f, RGBA(63));
		font.Render(to_string(totalGamePoints), Vec2(-ScrWidth(), ScrHeight() * 0.85f), ScrHeight() / 20.0f, RGBA(63, 63));
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