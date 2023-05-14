#include "Player.h"

Chunk::Chunk(iVec3 pos) :
	vector{}, pos(pos)
{
	memset(tiles, UnEnum(pos.z >= 0 ? TILE::AIR : TILE::ROCK), CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_WIDTH);
	for (int x = 0; x < CHUNK_WIDTH; x++)
		for (int y = 0; y < CHUNK_WIDTH; y++)
		{
			float tX = pos.x + x, tY = pos.y + y;
			float noise = game->planet->worldNoise.GetNoise(tX, tY) * 0.5f + 0.5f;
			noise = noise * 4.f + 0.5f; // 0.5-4.5
			if (noise > 3)
				noise = (noise - 3) * 5 + 3; // 3-10.5
			for (int z = 0; z < CHUNK_WIDTH; z++)
			{
				float noise2 = 0.f;// game->planet->caveNoise.GetNoise(float(pos.x + x), float(pos.y + y), float(pos.z + z));
				int tZ = z + pos.z;
				tiles[x][y][z] = UnEnum(noise < tZ || noise2 > 0.75f ? TILE::AIR : tZ > 7 ? TILE::SNOW : tZ > 3 ? TILE::ROCK : tZ > 2 ? TILE::SAND : tZ > 1 ?
					TILE::BAD_SOIL : tZ > 0 ? TILE::MID_SOIL : TILE::MAX_SOIL);
			}
		}
	GenerateMesh();
}

Planet::Planet()
{
	worldNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	worldNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
	worldNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2);
	worldNoise.SetFrequency(0.01f * (RandFloat() * 0.5f + 0.5f));
	worldNoise.SetFractalLacunarity(1.0f);
	worldNoise.SetFractalGain(0.25f);
	worldNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_Ridged);
	worldNoise.SetSeed(static_cast<int>(time(NULL)));

	friction = RandFloat() * 10 + 5;
	gravity = 10;

	dawnTime = RandFloat() * 59.99f + 0.01f; // Can't be 0 or it will crash.
	dayTime = RandFloat() * 60.0f;
	duskTime = RandFloat() * 60.0f;
	nightTime = RandFloat() * 60.0f;
	sunDir = Normalized(Vec3(RandCircPoint2(), -3.f));

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
	unique_ptr<Player> playerUnique = characters[UnEnum(settings.character)]->PClone();
	player = playerUnique.get();
	for (int i = 0; i < 100; i++)
	{
		if (!entities->OverlapsTile(player->pos, player->radius))
			break;
		else
			player->pos.z++;
	}
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
	
void Game::Update()
{
	inputs.UpdateMouse(window, zoom);

	if (uiMode == UIMODE::MAINMENU)
	{
		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Begin"))
		{
			Start();
			uiMode = UIMODE::CHARSELECT;
		}
		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, "Exit"))
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.625f)), ScrHeight() / 10.0f, IsFullscreen() ? "Unfullscreen" : "Fullscreen"))
			Fullscreen();
		else if (InputHoverSquare(iVec2(0, ScrHeight() / 2), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::EASY], settings.difficulty == DIFFICULTY::EASY ?
			RGBA(0, 255) : RGBA(255, 255, 255), settings.difficulty == DIFFICULTY::EASY ? RGBA(0, 127) : RGBA(127, 127, 127)))
		{
			settings.difficulty = DIFFICULTY::EASY;
			settings.Write();
		}
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.375f)), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::MEDIUM], settings.difficulty == DIFFICULTY::MEDIUM ?
			RGBA(255, 255) : RGBA(255, 255, 255), settings.difficulty == DIFFICULTY::MEDIUM ? RGBA(127, 127) : RGBA(127, 127, 127)))
		{
			settings.difficulty = DIFFICULTY::MEDIUM;
			settings.Write();
		}
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.25f)), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::HARD], settings.difficulty == DIFFICULTY::HARD ?
			RGBA(255) : RGBA(255, 255, 255), settings.difficulty == DIFFICULTY::HARD ? RGBA(127) : RGBA(127, 127, 127)))
		{
			settings.difficulty = DIFFICULTY::HARD;
			settings.Write();
		}
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.125f)), ScrHeight() / 10.0f, "Settings"))
			uiMode = UIMODE::SETTINGS;
	}
	else if (uiMode == UIMODE::SETTINGS)
	{
		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Return"))
			uiMode = UIMODE::MAINMENU;
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, settings.vSync ? "Yes vSync" : "No vSync", settings.vSync ? RGBA(0, 255) : RGBA(255),
			settings.vSync ? RGBA(0, 127) : RGBA(127)))
		{
			settings.vSync ^= true;
			settings.Write();
		}
		else if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.625f)), ScrHeight() / 10.0f,
			settings.colorBand ? "Yes color band" : "No color band", settings.colorBand ? RGBA(0, 255) : RGBA(255), settings.colorBand ? RGBA(0, 127) : RGBA(127)))
		{
			settings.colorBand ^= true;
			settings.Write();
		}
	}
	else if (uiMode == UIMODE::CHARSELECT)
	{
		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Begin"))
		{
			Start();
			uiMode = UIMODE::INGAME;
		}

		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, "Back") || inputs.escape.held)
			uiMode = UIMODE::MAINMENU;
		else
			for (int i = 0; i < characters.size(); i++)
				if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * (0.625f - i * 0.125f))), ScrHeight() / 10.0f, characters[i]->name,
					UnEnum(settings.character) == i ? characters[i]->color : RGBA(255, 255, 255), UnEnum(settings.character) == i ? characters[i]->color / 2 : RGBA(127, 127, 127)))
				{
					settings.character = static_cast<CHARS>(i); // Doesn't break so that it renders the rest as InputHoverSquare does rendering.
					settings.Write();
				}
	}
	else // In game or paused
	{
		if (playerAlive && inputs.escape.pressed)
			uiMode = uiMode == UIMODE::INGAME ? UIMODE::PAUSED : UIMODE::INGAME;

		if (playerAlive)
		{
			TUpdate();
			if (uiMode == UIMODE::PAUSED)
			{
				currentFramebuffer = TRUESCREEN;
				UseFramebuffer();
				inputs.UpdateKey(window, inputs.escape, GLFW_KEY_ESCAPE);
				if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Return"))
					uiMode = UIMODE::INGAME;
				if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, "Main menu"))
					uiMode = UIMODE::MAINMENU;
			}
		}
		else
		{
			if (currentFramebuffer != 0)
			{
				currentFramebuffer = 0;
				UseFramebuffer();
			}
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			font.Render(to_string(totalGamePoints) + " points", { -ScrWidth(), int(ScrHeight() * 0.75f) }, ScrHeight() / 5.0f, { 255, 255, 255 });
			font.Render("Killed by : ", { -ScrWidth(), int(ScrHeight() * 0.5f) }, ScrHeight() / 5.0f, { 255, 255, 255 });
			font.Render(deathCauseName, { -ScrWidth(), int(ScrHeight() * 0.25f) }, ScrHeight() / 5.0f, { 255, 255, 255 });

			if (InputHoverSquare(iVec2(0, ScrHeight() / 2), ScrHeight() / 10.0f, "Restart"))
			{
				Start();
				uiMode = UIMODE::INGAME;
			}
			if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.375f)), ScrHeight() / 10.0f, "Main menu"))
				uiMode = UIMODE::MAINMENU;

			font.Render(difficultyStrs[settings.difficulty], { -ScrWidth(), int(ScrHeight() * -0.5f) }, ScrHeight() / 5.0f,
				{ settings.difficulty == DIFFICULTY::EASY ? 0u : 255u, settings.difficulty == DIFFICULTY::HARD ? 0u : 255u });
		}
	}
}

void Game::ApplyLighting()
{
	JRGB ambientColor = planet->GetAmbient(brightness);

	currentFramebuffer = SHADOWMAP;
	UseFramebuffer();
	glBlendFunc(GL_ONE, GL_ONE);
	
	glDisable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(sunShader);
	glUniform3f(glGetUniformLocation(sunShader, "color"), ambientColor.r / 255.0f, ambientColor.g / 255.0f, ambientColor.b / 255.0f);
	glUniform3f(glGetUniformLocation(sunShader, "direction"), planet->sunDir.x, planet->sunDir.y, planet->sunDir.z);
	glUniform1i(glGetUniformLocation(sunShader, "colorBand"), int(settings.colorBand));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainScreen->normalBuffer);
	glUniform1i(glGetUniformLocation(sunShader, "normalMap"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mainScreen->positionBuffer);
	glUniform1i(glGetUniformLocation(sunShader, "positionMap"), 1);
	screenSpaceQuad.Draw();


	glUseProgram(shadowShader);
	glUniformMatrix4fv(glGetUniformLocation(shadowShader, "perspective"), 1, GL_FALSE, glm::value_ptr(perspective));
	glUniform1i(glGetUniformLocation(shadowShader, "colorBand"), int(settings.colorBand));
	glUniform1i(glGetUniformLocation(shadowShader, "normalMap"), 0);
	glUniform1i(glGetUniformLocation(shadowShader, "positionMap"), 1);

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
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
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
	if (dTime >= 1.f)
		return; // Don't run frame if there was a GIGANTIC frame drop.
	// This is normally off.
	glEnable(GL_DEPTH_TEST);
	// Prepare current framebuffer to be used in rendering of the frame.
	currentFramebuffer = MAINSCREEN;
	UseFramebuffer();
	// In TUpdate such that time doesn't progress whilst paused.
	if (game->uiMode == UIMODE::INGAME)
	{
		tTime += dTime;
		game->inputs.Update(window);
	}
	else
		dTime = 0;


	screenShake *= powf(0.25f, game->dTime);
	screenOffset = Vec3(Vec2(screenShkX.GetNoise(tTime, 0.f), screenShkY.GetNoise(tTime, 0.f)) * screenShake, zoom);

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

	entities->Update(); // Updates all entities.

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static const float normalUp[] = { 0, 0, 1, 0 };
	glClearBufferfv(GL_COLOR, 1, normalUp);
	static const float noPosition[] = { 0, 0, 0, 0 };
	glClearBufferfv(GL_COLOR, 2, noPosition);
	entities->DUpdate(); // Draws all entities.
	ApplyLighting(); // Apply lighting.
	glDisable(GL_DEPTH_TEST);
	DrawFramebufferOnto(TRUESCREEN);

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
	}

	lastPlayerPos = player->pos;

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