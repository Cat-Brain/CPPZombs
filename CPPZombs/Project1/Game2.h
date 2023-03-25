#include "Player.h"

Planet::Planet()
{
	worldNoise.SetFrequency(5.f);
	worldNoise.SetFractalLacunarity(2.0f);
	worldNoise.SetFractalGain(0.5f);
	worldNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
	worldNoise.SetSeed(static_cast<int>(time(NULL)));

	friction = RandFloat() * 10 + 5;

	dawnTime = RandFloat() * 59.99f + 0.01f; // Can't be 0 or it will crash.
	dayTime = RandFloat() * 60.0f;
	duskTime = RandFloat() * 60.0f;
	nightTime = RandFloat() * 60.0f;

	if (rand() % 2 == 0)
	{
		ambientDark = RandFloat();
		ambientLight = 1;
	}
	else
	{
		ambientDark = 0;
		ambientLight = RandFloat();
	}


	color1.r = rand() % 128 + 64;
	color1.g = rand() % 128 + 64;
	color1.b = rand() % 128 + 64;

	color2.r = color1.r + rand() % 32 + 32;
	color2.g = color1.g + rand() % 32 + 32;
	color2.b = color1.b + rand() % 32 + 32;

	fog.r = rand() % 8;
	fog.g = rand() % 8;
	fog.b = rand() % 8;

	enemies = make_unique<Enemies::Instance>(Enemies::naturalSpawns.RandomClone());
	bosses = make_unique<Enemies::Instance>(Enemies::spawnableBosses.RandomClone());
}

void Game::Start()
{
	srand(static_cast<uint>(time(NULL) % UINT_MAX));

	planet = std::make_unique<Planet>();

	entities = make_unique<Entities>();
	unique_ptr<Player> playerUnique = characters[selectedCharacter]->PClone();
	player = playerUnique.get();
	entities->push_back(std::move(playerUnique));
	playerAlive = true;
	totalGamePoints = 0;

	lastWave = tTime;
	waveCount = 0;
	shouldSpawnBoss = false;

	screenShkX.SetFrequency(5.f);
	screenShkX.SetFractalLacunarity(2.0f);
	screenShkX.SetFractalGain(0.5f);
	screenShkX.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
	screenShkX.SetSeed(static_cast<int>(time(NULL)));
	screenShkY = screenShkX;
	screenShkY.SetSeed(static_cast<int>(time(NULL) + 1));
	screenShake = 0.0f;
}

bool CheckInputSquareHover(Vec2 minPos, float scale, string text) // Returns if square is being hovered over.
{
	return game->inputs.screenMousePosition.x > minPos.x && game->inputs.screenMousePosition.x < minPos.x + font.TextWidth(text)* scale / font.minimumSize &&
		game->inputs.screenMousePosition.y > minPos.y + scale * font.mininumVertOffset / font.minimumSize &&
		game->inputs.screenMousePosition.y < minPos.y + scale * font.maxVertOffset / font.minimumSize;
}

bool InputHoverSquare(Vec2 minPos, float scale, string text, RGBA color1 = RGBA(255, 255, 255), RGBA color2 = RGBA(127, 127, 127)) // Renders and returns if this was clicked on(not hovered over).
{
		if (CheckInputSquareHover(minPos, scale, text))
		{
			font.Render(text, minPos * 2.f - Vec2(ScrDim()), scale * 2, color2);
			return game->inputs.leftMouse.pressed;
		}
		else
			font.Render(text, { minPos * 2.f - Vec2(ScrDim())}, scale * 2, color1);
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

		inputs.FindMousePos(window, zoom);

		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Begin"))
		{
			Start();
			uiMode = UIMODE::CHARSELECT;
		}
		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, "Exit"))
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.625f)), ScrHeight() / 10.0f, IsFullscreen() ? "Unfullscreen" : "Fullscreen"))
			Fullscreen();
		else if (InputHoverSquare(iVec2(0, ScrHeight() / 2), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::EASY], difficulty == DIFFICULTY::EASY ?
			RGBA(0, 255) : RGBA(255, 255, 255), difficulty == DIFFICULTY::EASY ? RGBA(0, 127) : RGBA(127, 127, 127)))
			difficulty = DIFFICULTY::EASY;
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.375f)), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::MEDIUM], difficulty == DIFFICULTY::MEDIUM ?
			RGBA(255, 255) : RGBA(255, 255, 255), difficulty == DIFFICULTY::MEDIUM ? RGBA(127, 127) : RGBA(127, 127, 127)))
			difficulty = DIFFICULTY::MEDIUM;
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.25f)), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::HARD], difficulty == DIFFICULTY::HARD ?
			RGBA(255) : RGBA(255, 255, 255), difficulty == DIFFICULTY::HARD ? RGBA(127) : RGBA(127, 127, 127)))
			difficulty = DIFFICULTY::HARD;
	}
	else if (uiMode == UIMODE::CHARSELECT)
	{
		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		inputs.FindMousePos(window, zoom);
		
		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Begin"))
		{
			Start();
			uiMode = UIMODE::INGAME;
		}

		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, "Back"))
			uiMode = UIMODE::MAINMENU;
		else
			for (int i = 0; i < characters.size(); i++)
				if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * (0.625f - i * 0.125f))), ScrHeight() / 10.0f, characters[i]->name,
					selectedCharacter == i ? characters[i]->color : RGBA(255, 255, 255), selectedCharacter == i ? characters[i]->color / 2 : RGBA(127, 127, 127)))
					selectedCharacter = static_cast<CHARS>(i); // Doesn't break so that it renders the rest as InputHoverSquare does rendering.
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

				inputs.FindMousePos(window, zoom);

				if (InputHoverSquare(iVec2(0, ScrHeight() / 2), ScrHeight() / 10.0f, "Restart"))
				{
					Start();
					uiMode = UIMODE::INGAME;
				}
				if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.375f)), ScrHeight() / 10.0f, "Main menu"))
					uiMode = UIMODE::MAINMENU;

				font.Render(difficultyStrs[difficulty], {-ScrWidth(), int(ScrHeight() * -0.5f)}, ScrHeight() / 5.0f,
					{difficulty == DIFFICULTY::EASY ? 0u : 255u, difficulty == DIFFICULTY::HARD ? 0u : 255u});
			}
		}
		if (uiMode == UIMODE::PAUSED)
		{
			currentFramebuffer = TRUESCREEN;
			UseFramebuffer();

			inputs.FindMousePos(window, zoom);
			if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Return"))
				uiMode = UIMODE::INGAME;
			if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, "Main menu"))
				uiMode = UIMODE::MAINMENU;
		}
	}
}

void Game::ApplyLighting()
{
	JRGB ambientColor = planet->GetAmbient(brightness);

	currentFramebuffer = SHADOWMAP;
	UseFramebuffer();
	glBlendFunc(GL_ONE, GL_ONE);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(sunShader);
	glUniform3f(glGetUniformLocation(sunShader, "color"), ambientColor.r / 255.0f, ambientColor.g / 255.0f, ambientColor.b / 255.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainScreen->normalBuffer);
	glUniform1i(glGetUniformLocation(sunShader, "normalMap"), 0);
	screenSpaceQuad.Draw();


	glUseProgram(shadowShader);
	glUniform1i(glGetUniformLocation(shadowShader, "normalMap"), 0);

	glUniform2i(glGetUniformLocation(shadowShader, "scrDim"),
		static_cast<int>(screenRatio * zoom * 2), static_cast<int>(zoom * 2));
	for (unique_ptr<LightSource>& light : entities->lightSources)
		DrawLight(light->pos, light->range, light->color);

	// If something is rendered it will subtract from the source, this will be used for dark sources.
	glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);

	for (unique_ptr<LightSource>& light : entities->darkSources)
		DrawLight(light->pos, light->range, light->color);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	currentFramebuffer = MAINSCREEN;
	UseFramebuffer();
	
	glUseProgram(shadingShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainScreen->textureColorbuffer);
	glUniform1i(glGetUniformLocation(shadingShader, "screenTexture"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowMap->textureColorbuffer);
	glUniform1i(glGetUniformLocation(shadingShader, "shadowTexture"), 1);

	screenSpaceQuad.Draw();
}

float Game::BrightnessAtPos(iVec2 pos)
{
	JRGB ambient = planet->GetAmbient(planet->GetBrightness());
	float r = ambient.r, g = ambient.g, b = ambient.b;
	for (unique_ptr<LightSource>& light : entities->lightSources)
		if (Squistance(pos, light->pos) < light->range)
		{
			float multiplier = 1.0f - Squistance(pos, light->pos) / light->range;
			r += light->color.r * multiplier;
			g += light->color.g * multiplier;
			b += light->color.b * multiplier;
		}
	for (unique_ptr<LightSource>& light : entities->darkSources)
		if (Squistance(pos, light->pos) < light->range)
		{
			float multiplier = 1.0f - Squistance(pos, light->pos) / light->range;
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
	inputs.FindMousePos(window, zoom);

	screenShake *= powf(0.25f, game->dTime);
	screenOffset = Vec2(screenShkX.GetNoise(tTime, 0.f), screenShkY.GetNoise(tTime, 0.f)) * screenShake;

	zoom = ClampF(zoom + float(int(inputs.e.held) - int(inputs.q.held)) * dTime * zoomSpeed, minZoom, maxZoom);

	brightness = planet->GetBrightness();

	// Spawn boss:
	if (inputs.enter.pressed && !shouldSpawnBoss)
	{
		shouldSpawnBoss = true;
		timeStartBossPrep = tTime;
	}

	if (shouldSpawnBoss && tTime - timeStartBossPrep >= 60.0f)
	{
		planet->bosses->SpawnOneRandom();
		shouldSpawnBoss = false;
	}
	waveCount += int(inputs.period.pressed) - int(inputs.comma.pressed);
	// New wave:
	if (tTime - lastWave > secondsBetweenWaves && frameCount != 0 || inputs.slash.pressed)
	{
		waveCount++;
		planet->enemies->SpawnRandomEnemies();
		lastWave = tTime;
	}

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	static const float normalUp[] = { 0, 0, 1, 0 };
	glClearBufferfv(GL_COLOR, 1, normalUp);
	glUseProgram(backgroundShader);
	glUniform2f(glGetUniformLocation(backgroundShader, "offset"), PlayerPos().x + screenOffset.x, PlayerPos().y + screenOffset.y);
	glUniform2f(glGetUniformLocation(backgroundShader, "screenDim"), screenRatio * zoom, zoom);
	glUniform3f(glGetUniformLocation(backgroundShader, "col1"), planet->color1.r / 255.0f, planet->color1.g / 255.0f, planet->color1.b / 255.0f);
	glUniform3f(glGetUniformLocation(backgroundShader, "col2"), planet->color2.r / 255.0f, planet->color2.g / 255.0f, planet->color2.b / 255.0f);
	screenSpaceQuad.Draw();

	entities->Update(); // Updates all entities.
	entities->DUpdate(); // Draws all entities.
	ApplyLighting(); // Apply lighting.
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
				std::to_string(int(timeTillNextBoss * 10) - int(timeTillNextBoss) * 10), iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.95f)), ScrHeight() / 20.0f,
				planet->enemies->superWave ? RGBA(255, 255) : RGBA(0, 255, 255));
		}
		else
			font.Render(std::to_string(int(timeTillNextWave)) + "." + std::to_string(int(timeTillNextWave * 10) - int(timeTillNextWave) * 10) + " - " + std::to_string(waveCount),
				iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.95f)), ScrHeight() / 20.0f, planet->enemies->superWave ? RGBA(255, 255) : RGBA(0, 255, 255));
		font.Render(std::to_string(player->health), iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.9f)), ScrHeight() / 20.0f, RGBA(63));
		font.Render(to_string(totalGamePoints), iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.85f)), ScrHeight() / 20.0f, RGBA(63, 63));
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