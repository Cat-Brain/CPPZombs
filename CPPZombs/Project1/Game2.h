#include "Planet.h"

void Game::Start()
{
	srand(static_cast<uint>(time(NULL)));

	entities = make_unique<Entities>();
	unique_ptr<Player> playerUnique = make_unique<Player>(vZero, vOne, 6, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), true, RGBA(), 20.0f, 1.0f, 10, 5, "Player");
	player = playerUnique.get();
	entities->push_back(std::move(playerUnique));
	playerAlive = true;
	totalGamePoints = 0;

	lastWave = tTime;
	waveCount = 0;
	shouldSpawnBoss = false;

	planet = std::make_unique<Planet>();
}

bool CheckInputSquareHover(Vec2 minPos, float scale, string text) // Returns if square is being hovered over.
{
	return game->inputs.screenMousePosition.x > minPos.x && game->inputs.screenMousePosition.x < minPos.x + font.TextWidth(text)* scale / font.minimumSize &&
		game->inputs.screenMousePosition.y > minPos.y + scale * font.mininumVertOffset / font.minimumSize &&
		game->inputs.screenMousePosition.y < minPos.y + scale * font.maxVertOffset / font.minimumSize;
}

bool InputHoverSquare(Vec2 minPos, float scale, string text) // Renders and returns if this was clicked on(not hovered over).
{
		if (CheckInputSquareHover(minPos, scale, text))
		{
			font.Render(text, minPos * 2 - ScrDim(), scale * 2, RGBA(127, 127, 127, 255));
			return game->inputs.leftMouse.pressed;
		}
		else
			font.Render(text, { minPos * 2 - ScrDim() }, scale * 2, RGBA(255, 255, 255, 255));
		return false;
}
	
void Game::Update()
{
	inputs.Update(window);
	if (uiMode == UIMODE::MAINMENU)
	{
		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		inputs.FindMousePos(window);

		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.875f), ScrHeight() / 10.0f, "Begin"))
		{
			Start();
			uiMode = UIMODE::INGAME;
		}
		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.75f), ScrHeight() / 10.0f, "Exit"))
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (InputHoverSquare(Vec2(0, ScrHeight() * 0.625f), ScrHeight() / 10.0f, IsFullscreen() ? "Unfullscreen" : "Fullscreen"))
			Fullscreen();
	}
	else
	{
		if (playerAlive && inputs.escape.pressed)
			uiMode = uiMode == UIMODE::INGAME ? UIMODE::PAUSED : UIMODE::INGAME;

		if (uiMode == UIMODE::INGAME)
		{
			if (playerAlive)
				TUpdate();
			else
			{
				if (currentFramebuffer != 0)
				{
					currentFramebuffer = 0;
					UseFramebuffer();
				}
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				font.Render(to_string(totalGamePoints) + " points", { -ScrWidth(), int(ScrHeight() * 0.75f) }, ScrHeight() / 5.0f, { 255, 255, 255 });
				font.Render("Killed by : ", { -ScrWidth(), int(ScrHeight() * 0.5f) }, ScrHeight() / 5.0f, { 255, 255, 255 });
				font.Render(deathCauseName, { -ScrWidth(), int(ScrHeight() * 0.25f) }, ScrHeight() / 5.0f, { 255, 255, 255 });

				inputs.FindMousePos(window);

				if (InputHoverSquare(Vec2(0, ScrHeight() * 0.5f), ScrHeight() / 10.0f, "Restart"))
				{
					Start();
					uiMode = UIMODE::INGAME;
				}
				if (InputHoverSquare(Vec2(0, ScrHeight() * 0.375f), ScrHeight() / 10.0f, "Main menu"))
					uiMode = UIMODE::MAINMENU;
			}
		}
		if (uiMode == UIMODE::PAUSED)
		{
			currentFramebuffer = 0;
			UseFramebuffer();

			inputs.FindMousePos(window);
			if (InputHoverSquare(Vec2(0, ScrHeight() * 0.875f), ScrHeight() / 10.0f, "Return"))
			{
				player->lastClick = tTime;
				uiMode = UIMODE::INGAME;
			}
			if (InputHoverSquare(Vec2(0, ScrHeight() * 0.75f), ScrHeight() / 10.0f, "Main menu"))
				uiMode = UIMODE::MAINMENU;
		}
	}
}

void Game::ApplyLighting()
{
	JRGB ambientColor = planet->GetAmbient(brightness);

	currentFramebuffer = 2;
	UseFramebuffer();
	glClearColor(planet->fog.r / 255.0f, planet->fog.g / 255.0f, planet->fog.b / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	entities->SubScatUpdate();

	currentFramebuffer = 3;
	UseFramebuffer();

	glUseProgram(shadowShader);
	glClearColor(ambientColor.r / 255.0f, ambientColor.g / 255.0f, ambientColor.b / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, subScat.textureColorbuffer);
	glUniform1i(glGetUniformLocation(shadowShader, "subScat"), 0);

	glUniform2i(glGetUniformLocation(shadowShader, "scrDim"),
		ScrWidth(), ScrHeight());

	for (unique_ptr<LightSource>& light : entities->lightSources)
	{
		glUniform1f(glGetUniformLocation(shadowShader, "range"), light->range);

		Vec2f scrPos = light->pos - PlayerPos();
		glUniform2f(glGetUniformLocation(shadowShader, "scale"),
			float(light->range * 4 + 2) / ScrWidth(), float(light->range * 4 + 2) / ScrHeight());

		glUniform2f(glGetUniformLocation(shadowShader, "position"),
			(scrPos.x - light->range) * 2 / ScrWidth(),
			(scrPos.y - light->range) * 2 / ScrHeight());

		glUniform2f(glGetUniformLocation(shadowShader, "center"), scrPos.x, scrPos.y);

		glUniform2f(glGetUniformLocation(shadowShader, "bottomLeft"),
			scrPos.x - light->range, scrPos.y - light->range);

		glUniform2f(glGetUniformLocation(shadowShader, "topRight"),
			scrPos.x + light->range, scrPos.y + light->range);

		glUniform3f(glGetUniformLocation(shadowShader, "color"), light->color.r / 255.0f, light->color.g / 255.0f, light->color.b / 255.0f);

		quad.Draw();
	}

	// If something is rendered it will subtract from the source, this will be used for dark sources.
	glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);

	for (unique_ptr<LightSource>& light : entities->darkSources)
	{
		glUniform1f(glGetUniformLocation(shadowShader, "range"), light->range);

		Vec2f scrPos = light->pos - PlayerPos();
		glUniform2f(glGetUniformLocation(shadowShader, "scale"),
			float(light->range * 4 + 2) / ScrWidth(), float(light->range * 4 + 2) / ScrHeight());

		glUniform2f(glGetUniformLocation(shadowShader, "position"),
			(scrPos.x - light->range) * 2 / ScrWidth(),
			(scrPos.y - light->range) * 2 / ScrHeight());

		glUniform2f(glGetUniformLocation(shadowShader, "center"), scrPos.x, scrPos.y);

		glUniform2f(glGetUniformLocation(shadowShader, "bottomLeft"),
			scrPos.x - light->range, scrPos.y - light->range);

		glUniform2f(glGetUniformLocation(shadowShader, "topRight"),
			scrPos.x + light->range, scrPos.y + light->range);

		glUniform3f(glGetUniformLocation(shadowShader, "color"), light->color.r / 255.0f, light->color.g / 255.0f, light->color.b / 255.0f);

		quad.Draw();
	}

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	currentFramebuffer = 1;
	UseFramebuffer();
	
	glUseProgram(shadingShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, midRes.textureColorbuffer);
	glUniform1i(glGetUniformLocation(shadingShader, "screenTexture"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowMap.textureColorbuffer);
	glUniform1i(glGetUniformLocation(shadingShader, "shadowTexture"), 1);

	screenSpaceQuad.Draw();
}

float Game::BrightnessAtPos(Vec2 pos)
{
	JRGB ambient = planet->GetAmbient(planet->GetBrightness());
	float r = ambient.r, g = ambient.g, b = ambient.b;
	for (unique_ptr<LightSource>& light : entities->lightSources)
		if (pos.Squistance(light->pos) < light->range)
		{
			float multiplier = 1.0f - pos.Squistance(light->pos) / light->range;
			r += light->color.r * multiplier;
			g += light->color.g * multiplier;
			b += light->color.b * multiplier;
		}
	for (unique_ptr<LightSource>& light : entities->darkSources)
		if (pos.Squistance(light->pos) < light->range)
		{
			float multiplier = 1.0f - pos.Squistance(light->pos) / light->range;
			r -= light->color.r * multiplier;
			g -= light->color.g * multiplier;
			b -= light->color.b * multiplier;
		}
	return static_cast<float>(ClampF(r + g + b, 0, 765)) / 765.0f; // 765 = 255 * 3 and 255 is max byte and 3 is channel count.
}

void Game::TUpdate()
{
	// Prepare current framebuffer to be used in rendering of the frame.
	currentFramebuffer = 1;
	UseFramebuffer();
	// In TUpdate such that time doesn't progress whilst paused.
	tTime += dTime;
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
	waveCount += int(inputs.period.pressed) - int(inputs.comma.pressed);
	// New wave:
	if (tTime - lastWave > secondsBetweenWaves && frameCount != 0 || inputs.slash.pressed)
	{
		planet->enemies.SpawnRandomEnemies();
		lastWave = tTime;
		waveCount++;
	}

	glUseProgram(backgroundShader);
	glUniform2f(glGetUniformLocation(backgroundShader, "offset"), PlayerPos().x * 2.0f, PlayerPos().y * 2.0f);
	glUniform2f(glGetUniformLocation(backgroundShader, "screenDim"), static_cast<float>(ScrWidth()), static_cast<float>(ScrHeight()));
	glUniform3f(glGetUniformLocation(backgroundShader, "col1"), planet->color1.r / 255.0f, planet->color1.g / 255.0f, planet->color1.b / 255.0f);
	glUniform3f(glGetUniformLocation(backgroundShader, "col2"), planet->color2.r / 255.0f, planet->color2.g / 255.0f, planet->color2.b / 255.0f);
	screenSpaceQuad.Draw();

	entities->Update(); // Updates all entities.
	entities->DUpdate(); // Draws all entities.
	ApplyLighting(); // Apply lighting.
	Draw(inputs.mousePosition + player->pos, RGBA(0, 0, 0, static_cast<uint8_t>((sinf(tTime * 3.14f * 3.0f) + 1.0f) * 64))); // Draw mouse.
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
			font.Render(std::to_string(int(timeTillNextWave)) + "." +
				std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount) + " " +
				std::to_string(int(timeTillNextBoss)) + "." +
				std::to_string(int(timeTillNextBoss * 10) - int(timeTillNextBoss) * 10), Vec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.95f)), ScrHeight() / 20.0f, RGBA(0, 255, 255));
		}
		else
			font.Render(std::to_string(int(timeTillNextWave)) + "." + std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount),
				Vec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.95f)), ScrHeight() / 20.0f, RGBA(0, 255, 255));
		font.Render(std::to_string(player->health), Vec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.9f)), ScrHeight() / 20.0f, RGBA(63));
		font.Render(to_string(totalGamePoints), Vec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.85f)), ScrHeight() / 20.0f, RGBA(63, 63));
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