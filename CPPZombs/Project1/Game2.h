#include "Player.h"

vector<RGBA> logBookColors = {
	RGBA(255, 127, 127),
	RGBA(255, 191, 127),
	RGBA(255, 255, 127),
	RGBA(127, 255, 127),
	RGBA(127, 127, 255),
	RGBA(255, 127, 255)
};
void LogBook::DUpdate()
{
	switch (logMode)
	{
	case LOGMODE::MAIN:
	{
		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Return"))
			isOpen = false;
		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, "Controls"))
			logMode = LOGMODE::CONTROLS;
		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.625f)), ScrHeight() / 10.0f, "Plants"))
			logMode = LOGMODE::PLANTS;
		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.5f)), ScrHeight() / 10.0f, "Towers"))
			logMode = LOGMODE::TOWERS;
		break;
	}
	case LOGMODE::CONTROLS:
	{
		vector<string> renderStrings = {
			"WASD for movement",
			"Left click shoots and plants",
			"Right click shoots from offhand",
			"R and shift do skills",
			"Skills are character specific"
			"",
			"Plants grow best on soil",
			"Soil is by default brown",
			"Quartz improves soil",
			"Ruby creates super soil",
			"",
			"Tab for inventory",
			"Q and E to switch row",
			"Row switching is a setting",
			"",
			"V to enter build mode",
			"Left click to place",
			"Right click to destroy",
			"",
			"U to disable or enable UI",
			"Left control to enable",
			"or diable vacuuming"
		};
		float scale = ScrHeight() * 0.125f;
		int rows = renderStrings.size() + 1;
		scroll = ClampF(scroll - game->inputs.mouseScrollF, 0, max(0, rows - 8));
		float offset = scroll * scale;
		int i = 0;
		for (string str : renderStrings)
			font.Render(str, iVec2(-ScrWidth(), static_cast<int>(offset * 2 + ScrHeight() * (0.5f - i++ * 0.25f))),
				ScrHeight() * 0.2f, logBookColors[i % logBookColors.size()]);

		if (InputHoverSquare(iVec2(0, static_cast<int>(offset + ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Return"))
			logMode = LOGMODE::MAIN;
		break;
	}
	case LOGMODE::PLANTS:
	{
		if (individualIndex == -1)
		{
			float scale = ScrHeight() * 0.125f;
			int rows = Plants::plants.size() + 1;
			scroll = ClampF(scroll - game->inputs.mouseScrollF, 0, rows - 8);
			float offset = scroll * scale;
			if (InputHoverSquare(iVec2(0, static_cast<int>(offset + ScrHeight() * 0.875f)), ScrHeight() * 0.1f, "Return"))
				logMode = LOGMODE::MAIN;
			for (int i = 0; i < Plants::plants.size(); i++)
				if (InputHoverSquare(iVec2(0, static_cast<int>(offset + ScrHeight() * (0.75f - i * 0.125f))), ScrHeight() * 0.1f, Plants::plants[i]->name,
					Plants::plants[i]->color, Plants::plants[i]->color / 2))
				{
					individualIndex = i;
				}
		}
		else
		{
			Shrub* plant = Plants::plants[individualIndex];
			int i = 0;
			vector<string> displayStrings = plant->DisplayStrings();
			for (string str : displayStrings)
				font.Render(str, iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * (0.5f - i++ * 0.25f))),
					ScrHeight() / 5.f, logBookColors[i % logBookColors.size()]);

			if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Return"))
				individualIndex = -1;
		}
		break;
	}
	case LOGMODE::TOWERS:
	{
		if (individualIndex == -1)
		{
			float scale = ScrHeight() * 0.125f;
			int rows = Defences::Towers::towers.size() + 1;
			scroll = ClampF(scroll - game->inputs.mouseScrollF, 0, rows - 8);
			float offset = scroll * scale;
			if (InputHoverSquare(Vec2(0, offset + ScrHeight() * 0.875f), ScrHeight() * 0.1f, "Return"))
				logMode = LOGMODE::MAIN;
			for (int i = 0; i < Defences::Towers::towers.size(); i++)
				if (InputHoverSquare(Vec2(0, offset + ScrHeight() * (0.75f - i * 0.125f)), ScrHeight() * 0.1f, Defences::Towers::towers[i]->name,
					Defences::Towers::towers[i]->color, Defences::Towers::towers[i]->color / 2))
					individualIndex = i;
		}
		else
		{
			Tower* tower = Defences::Towers::towers[individualIndex];
			vector<string> renderStrings;
			vector<RGBA> renderColors;
			for (ItemInstance itemInst : tower->recipe)
			{
				renderStrings.push_back(to_string(itemInst.count) + " " + itemInst->name);
				renderColors.push_back(itemInst->color);
			}

			split(*tower->description, '\n', renderStrings);

			float scale = ScrHeight() * 0.125f;
			int rows = renderStrings.size() + 1;
			scroll = ClampF(scroll - game->inputs.mouseScrollF, 0, max(0, rows - 8));
			float offset = scroll * scale;
			int i = 0;
			for (string str : renderStrings)
				font.Render(str, iVec2(-ScrWidth(), static_cast<int>(offset * 2 + ScrHeight() * (0.5f - i++ * 0.25f))),
					ScrHeight() * 0.2f, i < renderColors.size() ? renderColors[i] : logBookColors[(i - renderColors.size()) % logBookColors.size()]);

			if (InputHoverSquare(iVec2(0, static_cast<int>(offset + ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Return"))
				individualIndex = -1;
		}
		break;
	}
	// Settings:
	case LOGMODE::SETTINGS:
	{
		float scale = ScrHeight() * 0.1f;
		int i = 1;
		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int rows = game->settings.dispBoolSettings.size() + 3;
		scroll = ClampF(scroll - game->inputs.mouseScrollF, 0, rows - 8);
		float offset = scroll * ScrHeight() * 0.125;

		if (InputHoverSquare(Vec2(0, offset + ScrHeight() * (1 - 0.125f * i++)), scale, "Return"))
			isOpen = false;

		if (InputHoverSquare(Vec2(0, offset + ScrHeight() * (1 - 0.125 * i++)), scale, game->IsFullscreen() ? "Unfullscreen" : "Fullscreen"))
			game->Fullscreen();

		float horOffset = font.TextWidthTrue("Chunk Render Dist =  " + to_string(game->settings.chunkRenderDist)) * scale;
		font.Render("Chunk Render Dist = " + to_string(game->settings.chunkRenderDist), Vec2(-ScrWidth(), offset * 2 + ScrHeight() * (2 - 0.25 * i) - ScrHeight()), scale * 2, RGBA(255, 255, 255));
		if (InputHoverSquare(Vec2(horOffset, offset + ScrHeight() * (1 - 0.125 * i)), scale,
			"\\/", RGBA(255, 255, 255), RGBA(127, 127, 127)))
		{
			game->settings.chunkRenderDist--;
			game->settings.chunkRenderDist = max(1u, game->settings.chunkRenderDist);
			game->settings.Write();
		}
		horOffset += font.TextWidthTrue("\\/ ") * scale;
		if (InputHoverSquare(Vec2(horOffset, offset + ScrHeight() * (1 - 0.125 * i++)), scale,
			"/\\", RGBA(255, 255, 255), RGBA(127, 127, 127)))
		{
			game->settings.chunkRenderDist++;
			game->settings.Write();
		}

		for (std::pair<bool*, string> boolSetting : game->settings.dispBoolSettings)
			if (InputHoverSquare(Vec2(0, offset + ScrHeight() * (1 - 0.125f * i++)), ScrHeight() * 0.1f, boolSetting.second + " = " + ToStringBool(*boolSetting.first),
				*boolSetting.first ? RGBA(127, 255, 127) : RGBA(255, 127, 127), *boolSetting.first ? RGBA(63, 127, 63) : RGBA(127, 63, 63)))
			{
				*boolSetting.first ^= true;
				game->settings.Write();
			}
	}
	}
	game->inputs.mouseScrollF = 0;
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

	sunDir = Normalized(Vec3(RandCircPoint2(), -3.f));

	dawnTime  = 20;
	dayTime   = 80;
	duskTime  = 40;
	nightTime = 40;

	ambientDark = 0;
	ambientLight = 0.9f;


	color1.r = rand() % 128 + 64;
	color1.g = rand() % 128 + 64;
	color1.b = rand() % 128 + 64;

	color2.r = color1.r + rand() % 32 + 32;
	color2.g = color1.g + rand() % 32 + 32;
	color2.b = color1.b + rand() % 32 + 32;

	fog.r = rand() % 8;
	fog.g = rand() % 8;
	fog.b = rand() % 8;

	bosses = make_unique<Enemies::Instance>(Enemies::spawnableBosses.RandomClone());

	faction1Spawns = make_unique<Enemies::OvertimeInstance>(Enemies::faction1Spawns.RandomClone());
	faction1Spawns->Randomize();
	wildSpawns = make_unique<Enemies::OvertimeInstance>(Enemies::wildSpawns.RandomClone());
	wildSpawns->Randomize();
}

Chunk::Chunk(iVec3 pos) :
	vector{}, pos(pos), zPlus(-1), zMin(-1), yPlus(-1), yMin(-1), xPlus(-1), xMin(-1)
{
	memset(tiles, UnEnum(TILE::AIR), CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_WIDTH);
	for (int x = 0; x < CHUNK_WIDTH; x++)
		for (int y = 0; y < CHUNK_WIDTH; y++)
		{
			float tX = float(pos.x + x), tY = float(pos.y + y);
			float noise = game->planet->worldNoise.GetNoise(tX, tY) * 0.5f + 0.5f;
			
			noise = noise * 4.f + 0.5f; // 0.5-4.5
			if (noise > 3)
				noise = (noise - 3) * 5 + 3; // 3-10.5
			for (int z = 0; z < CHUNK_WIDTH; z++)
			{
				float noise2 = 0.f;// game->planet->caveNoise.GetNoise(float(pos.x + x), float(pos.y + y), float(pos.z + z));
				int tZ = z + pos.z;
				tiles[x][y][z] = UnEnum(noise < tZ || noise2 > 0.75f ? TILE::AIR : tZ > 7 ? TILE::SNOW : tZ > 3 ?
					TILE::ROCK : tZ > 2 ? TILE::SAND : TILE::BAD_SOIL);
			}
		}
}

void Chunk::Finalize()
{
	int index, thisIndex = static_cast<int>(game->entities->chunks.size() - 1);
	// zPlus
	if ((index = game->entities->ChunkAtPos(pos + upI * CHUNK_WIDTH)) != -1)
	{
		zPlus = index;
		game->entities->chunks[index].zMin = thisIndex;
		game->entities->chunks[index].RegenerateMesh();
	}
	// zMin
	if ((index = game->entities->ChunkAtPos(pos + downI * CHUNK_WIDTH)) != -1)
	{
		zMin = index;
		game->entities->chunks[index].zPlus = thisIndex;
		game->entities->chunks[index].RegenerateMesh();
	}
	// yPlus
	if ((index = game->entities->ChunkAtPos(pos + northI * CHUNK_WIDTH)) != -1)
	{
		yPlus = index;
		game->entities->chunks[index].yMin = thisIndex;
		game->entities->chunks[index].RegenerateMesh();
	}
	// yMin
	if ((index = game->entities->ChunkAtPos(pos + southI * CHUNK_WIDTH)) != -1)
	{
		yMin = index;
		game->entities->chunks[index].yPlus = thisIndex;
		game->entities->chunks[index].RegenerateMesh();
	}
	// xPlus
	if ((index = game->entities->ChunkAtPos(pos + westI * CHUNK_WIDTH)) != -1)
	{
		xPlus = index;
		game->entities->chunks[index].xMin = thisIndex;
		game->entities->chunks[index].RegenerateMesh();
	}
	// xMin
	if ((index = game->entities->ChunkAtPos(pos + eastI * CHUNK_WIDTH)) != -1)
	{
		xMin = index;
		game->entities->chunks[index].xPlus = thisIndex;
		game->entities->chunks[index].RegenerateMesh();
	}
	GenerateMesh();
}

void Chunk::GenerateMesh()
{
	bool allEmpty = true;
	for (byte x = 0; x < CHUNK_WIDTH; x++)
		for (byte y = 0; y < CHUNK_WIDTH; y++)
			for (byte z = 0; z < CHUNK_WIDTH; z++)
				allEmpty &= tiles[x][y][z] == 0;
	if (allEmpty)
	{
		indCount = 0;
		return;
	}
	vector<float> data{}; // Goes pos.x, pos.y, pos.z, color.r, color.g, color.b, normal.x, normal.y, normal.z
	for (byte x = 1; x < CHUNK_WIDTH - 1; x++)
		for (byte y = 1; y < CHUNK_WIDTH - 1; y++)
			for (byte z = 1; z < CHUNK_WIDTH - 1; z++)
				GenTile(data, x, y, z,
					tiles[x][y][z + 1] == 0,
					tiles[x][y][z - 1] == 0,
					tiles[x][y + 1][z] == 0,
					tiles[x][y - 1][z] == 0,
					tiles[x + 1][y][z] == 0,
					tiles[x - 1][y][z] == 0);

#pragma region face cases
#pragma region z
	if (zPlus != -1)
		for (byte x = 1; x < CHUNK_WIDTH - 1; x++)
			for (byte y = 1; y < CHUNK_WIDTH - 1; y++)
				GenTile(data, x, y, CHUNK_WIDTH - 1,
					game->entities->chunks[zPlus].tiles[x][y][0] == 0,
					tiles[x][y][CHUNK_WIDTH - 2] == 0,
					tiles[x][y + 1][CHUNK_WIDTH - 1] == 0,
					tiles[x][y - 1][CHUNK_WIDTH - 1] == 0,
					tiles[x + 1][y][CHUNK_WIDTH - 1] == 0,
					tiles[x - 1][y][CHUNK_WIDTH - 1] == 0);
	if (zMin != -1)
		for (byte x = 1; x < CHUNK_WIDTH - 1; x++)
			for (byte y = 1; y < CHUNK_WIDTH - 1; y++)
				GenTile(data, x, y, 0,
					tiles[x][y][1] == 0,
					game->entities->chunks[zMin].tiles[x][y][CHUNK_WIDTH - 1] == 0,
					tiles[x][y + 1][0] == 0,
					tiles[x][y - 1][0] == 0,
					tiles[x + 1][y][0] == 0,
					tiles[x - 1][y][0] == 0);
#pragma endregion
#pragma region y
	if (yPlus != -1)
		for (byte x = 1; x < CHUNK_WIDTH - 1; x++)
			for (byte z = 1; z < CHUNK_WIDTH - 1; z++)
				GenTile(data, x, CHUNK_WIDTH - 1, z,
					tiles[x][CHUNK_WIDTH - 1][z + 1] == 0,
					tiles[x][CHUNK_WIDTH - 1][z - 1] == 0,
					game->entities->chunks[yPlus].tiles[x][0][z] == 0,
					tiles[x][CHUNK_WIDTH - 2][z] == 0,
					tiles[x + 1][CHUNK_WIDTH - 1][z] == 0,
					tiles[x - 1][CHUNK_WIDTH - 1][z] == 0);
	if (yMin != -1)
		for (byte x = 1; x < CHUNK_WIDTH - 1; x++)
			for (byte z = 1; z < CHUNK_WIDTH - 1; z++)
				GenTile(data, x, 0, z,
					tiles[x][0][z + 1] == 0,
					tiles[x][0][z - 1] == 0,
					tiles[x][1][z] == 0,
					game->entities->chunks[yMin].tiles[x][CHUNK_WIDTH - 1][z] == 0,
					tiles[x + 1][0][z] == 0,
					tiles[x - 1][0][z] == 0);
#pragma endregion
#pragma region x
	if (xPlus != -1)
		for (byte y = 1; y < CHUNK_WIDTH - 1; y++)
			for (byte z = 1; z < CHUNK_WIDTH - 1; z++)
				GenTile(data, CHUNK_WIDTH - 1, y, z,
					tiles[CHUNK_WIDTH - 1][y][z + 1] == 0,
					tiles[CHUNK_WIDTH - 1][y][z - 1] == 0,
					tiles[CHUNK_WIDTH - 1][y + 1][z] == 0,
					tiles[CHUNK_WIDTH - 1][y - 1][z] == 0,
					game->entities->chunks[xPlus].tiles[0][y][z] == 0,
					tiles[CHUNK_WIDTH - 2][y][z] == 0);
	if (xMin != -1)
		for (byte y = 1; y < CHUNK_WIDTH - 1; y++)
			for (byte z = 1; z < CHUNK_WIDTH - 1; z++)
				GenTile(data, 0, y, z,
					tiles[0][y][z + 1] == 0,
					tiles[0][y][z - 1] == 0,
					tiles[0][y + 1][z] == 0,
					tiles[0][y - 1][z] == 0,
					tiles[1][y][z] == 0,
					game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][y][z] == 0);
#pragma endregion
#pragma endregion
#pragma region edge cases
#pragma region xy
	if (xMin != -1 || yMin != -1 || xPlus != -1 || yPlus != -1)
	{
		for (int z = 1; z < CHUNK_WIDTH - 1; z++)
		{
			// (min, min)
			if (xMin != -1 && yMin != -1)
				GenTile(data, 0, 0, z,
					tiles[0][0][z + 1] == 0,
					tiles[0][0][z - 1] == 0,
					tiles[0][1][z] == 0,
					game->entities->chunks[yMin].tiles[0][CHUNK_WIDTH - 1][z] == 0,
					tiles[1][0][z] == 0,
					game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][0][z] == 0);
			// (min, plus)
			if (xMin != -1 && yPlus != -1)
				GenTile(data, 0, CHUNK_WIDTH - 1, z,
					tiles[0][CHUNK_WIDTH - 1][z + 1] == 0,
					tiles[0][CHUNK_WIDTH - 1][z - 1] == 0,
					game->entities->chunks[yPlus].tiles[0][0][z] == 0,
					tiles[0][CHUNK_WIDTH - 2][z] == 0,
					tiles[1][CHUNK_WIDTH - 1][z] == 0,
					game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][z] == 0);
			// (plus, min)
			if (xPlus != -1 && yMin != -1)
				GenTile(data, CHUNK_WIDTH - 1, 0, z,
					tiles[CHUNK_WIDTH - 1][0][z + 1] == 0,
					tiles[CHUNK_WIDTH - 1][0][z - 1] == 0,
					tiles[CHUNK_WIDTH - 1][1][z] == 0,
					game->entities->chunks[yMin].tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][z] == 0,
					game->entities->chunks[xPlus].tiles[0][0][z] == 0,
					tiles[CHUNK_WIDTH - 2][0][z] == 0);
			// (plus, plus)
			if (xPlus != -1 && yPlus != -1)
				GenTile(data, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1, z,
					tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][z + 1] == 0,
					tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][z - 1] == 0,
					game->entities->chunks[yPlus].tiles[CHUNK_WIDTH - 1][0][z] == 0,
					tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 2][z] == 0,
					game->entities->chunks[xPlus].tiles[0][CHUNK_WIDTH - 1][z] == 0,
					tiles[CHUNK_WIDTH - 2][CHUNK_WIDTH - 1][z] == 0);
		}
	}
#pragma endregion
#pragma region xz
	if (xMin != -1 || zMin != -1 || xPlus != -1 || zPlus != -1)
	{
		for (int y = 1; y < CHUNK_WIDTH - 1; y++)
		{
			// (min, min)
			if (xMin != -1 && zMin != -1)
				GenTile(data, 0, y, 0,
					tiles[0][y][1] == 0,
					game->entities->chunks[zMin].tiles[0][y][CHUNK_WIDTH - 1] == 0,
					tiles[0][y + 1][0] == 0,
					tiles[0][y - 1][0] == 0,
					tiles[1][y][0] == 0,
					game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][y][0] == 0);
			// (min, plus)
			if (xMin != -1 && zPlus != -1)
				GenTile(data, 0, y, CHUNK_WIDTH - 1,
					game->entities->chunks[zPlus].tiles[0][y][0] == 0,
					tiles[0][y][CHUNK_WIDTH - 2] == 0,
					tiles[0][y + 1][CHUNK_WIDTH - 1] == 0,
					tiles[0][y - 1][CHUNK_WIDTH - 1] == 0,
					tiles[1][y][CHUNK_WIDTH - 1] == 0,
					game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][y][CHUNK_WIDTH - 1] == 0);
			// (plus, min)
			if (xPlus != -1 && zMin != -1)
				GenTile(data, CHUNK_WIDTH - 1, y, 0,
					tiles[CHUNK_WIDTH - 1][y][1] == 0,
					game->entities->chunks[zMin].tiles[CHUNK_WIDTH - 1][y][CHUNK_WIDTH - 1] == 0,
					tiles[CHUNK_WIDTH - 1][y + 1][0] == 0,
					tiles[CHUNK_WIDTH - 1][y - 1][0] == 0,
					game->entities->chunks[xPlus].tiles[0][y][0] == 0,
					tiles[CHUNK_WIDTH - 2][y][0] == 0);
			// (plus, plus)
			if (xPlus != -1 && zPlus != -1)
				GenTile(data, CHUNK_WIDTH - 1, y, CHUNK_WIDTH - 1,
					game->entities->chunks[zPlus].tiles[CHUNK_WIDTH - 1][y][0] == 0,
					tiles[CHUNK_WIDTH - 1][y][CHUNK_WIDTH - 2] == 0,
					tiles[CHUNK_WIDTH - 1][y + 1][CHUNK_WIDTH - 1] == 0,
					tiles[CHUNK_WIDTH - 1][y - 1][CHUNK_WIDTH - 1] == 0,
					game->entities->chunks[xPlus].tiles[0][y][CHUNK_WIDTH - 1] == 0,
					tiles[CHUNK_WIDTH - 2][y][CHUNK_WIDTH - 1] == 0);
		}
	}
#pragma endregion
#pragma region yz
	if (yMin != -1 || zMin != -1 || yPlus != -1 || zPlus != -1)
	{
		for (int x = 1; x < CHUNK_WIDTH - 1; x++)
		{
			// (min, min)
			if (yMin != -1 && zMin != -1)
				GenTile(data, x, 0, 0,
					tiles[x][0][1] == 0,
					game->entities->chunks[zMin].tiles[x][0][CHUNK_WIDTH - 1] == 0,
					tiles[x][1][0] == 0,
					game->entities->chunks[yMin].tiles[x][CHUNK_WIDTH - 1][0] == 0,
					tiles[x + 1][0][0] == 0,
					tiles[x - 1][0][0] == 0);
			// (min, plus)
			if (yMin != -1 && zPlus != -1)
				GenTile(data, x, 0, CHUNK_WIDTH - 1,
					game->entities->chunks[zPlus].tiles[x][0][0] == 0,
					tiles[x][0][CHUNK_WIDTH - 2] == 0,
					tiles[x][1][CHUNK_WIDTH - 1] == 0,
					game->entities->chunks[yMin].tiles[x][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
					tiles[x + 1][0][CHUNK_WIDTH - 1] == 0,
					tiles[x - 1][0][CHUNK_WIDTH - 1] == 0);
			// (plus, min)
			if (yPlus != -1 && zMin != -1)
				GenTile(data, x, CHUNK_WIDTH - 1, 0,
					tiles[x][CHUNK_WIDTH - 1][1] == 0,
					game->entities->chunks[zMin].tiles[x][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
					game->entities->chunks[yPlus].tiles[x][0][0] == 0,
					tiles[x][CHUNK_WIDTH - 2][0] == 0,
					tiles[x + 1][CHUNK_WIDTH - 1][0] == 0,
					tiles[x - 1][CHUNK_WIDTH - 1][0] == 0);
			// (plus, plus)
			if (yPlus != -1 && zPlus != -1)
				GenTile(data, x, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1,
					game->entities->chunks[zPlus].tiles[x][CHUNK_WIDTH - 1][0] == 0,
					tiles[x][CHUNK_WIDTH - 1][CHUNK_WIDTH - 2] == 0,
					game->entities->chunks[yPlus].tiles[x][0][CHUNK_WIDTH - 1] == 0,
					tiles[x][CHUNK_WIDTH - 2][CHUNK_WIDTH - 1] == 0,
					tiles[x + 1][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
					tiles[x - 1][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0);
		}
	}
#pragma endregion
#pragma endregion
#pragma region corner cases
	if (xPlus != -1 && yPlus != -1 && zPlus != -1)
		GenTile(data, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1,
			game->entities->chunks[zPlus].tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][0] == 0,
			tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][CHUNK_WIDTH - 2] == 0,
			game->entities->chunks[yPlus].tiles[CHUNK_WIDTH - 1][0][CHUNK_WIDTH - 1] == 0,
			tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 2][CHUNK_WIDTH - 1] == 0,
			game->entities->chunks[xPlus].tiles[0][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
			tiles[CHUNK_WIDTH - 2][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0);
	if (xPlus != -1 && yPlus != -1 && zMin != -1)
		GenTile(data, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1, 0,
			tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][1] == 0,
			game->entities->chunks[zMin].tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
			game->entities->chunks[yPlus].tiles[CHUNK_WIDTH - 1][0][0] == 0,
			tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 2][0] == 0,
			game->entities->chunks[xPlus].tiles[0][CHUNK_WIDTH - 1][0] == 0,
			tiles[CHUNK_WIDTH - 2][CHUNK_WIDTH - 1][0] == 0);
	if (xPlus != -1 && yMin != -1 && zPlus != -1)
		GenTile(data, CHUNK_WIDTH - 1, 0, CHUNK_WIDTH - 1,
			game->entities->chunks[zPlus].tiles[CHUNK_WIDTH - 1][0][0] == 0,
			tiles[CHUNK_WIDTH - 1][0][CHUNK_WIDTH - 2] == 0,
			tiles[CHUNK_WIDTH - 1][1][CHUNK_WIDTH - 1] == 0,
			game->entities->chunks[yMin].tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
			game->entities->chunks[xPlus].tiles[0][0][CHUNK_WIDTH - 1] == 0,
			tiles[CHUNK_WIDTH - 2][0][CHUNK_WIDTH - 1] == 0);
	if (xPlus != -1 && yMin != -1 && zMin != -1)
		GenTile(data, CHUNK_WIDTH - 1, 0, 0,
			tiles[CHUNK_WIDTH - 1][0][1] == 0,
			game->entities->chunks[zMin].tiles[CHUNK_WIDTH - 1][0][CHUNK_WIDTH - 1] == 0,
			tiles[CHUNK_WIDTH - 1][1][0] == 0,
			game->entities->chunks[yMin].tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][0] == 0,
			game->entities->chunks[xPlus].tiles[0][0][0] == 0,
			tiles[CHUNK_WIDTH - 2][0][0] == 0);
	if (xMin != -1 && yPlus != -1 && zPlus != -1)
		GenTile(data, 0, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1,
			game->entities->chunks[zPlus].tiles[0][CHUNK_WIDTH - 1][0] == 0,
			tiles[0][CHUNK_WIDTH - 1][CHUNK_WIDTH - 2] == 0,
			game->entities->chunks[yPlus].tiles[0][0][CHUNK_WIDTH - 1] == 0,
			tiles[0][CHUNK_WIDTH - 2][CHUNK_WIDTH - 1] == 0,
			tiles[1][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
			game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0);
	if (xMin != -1 && yPlus != -1 && zMin != -1)
		GenTile(data, 0, CHUNK_WIDTH - 1, 0,
			tiles[0][CHUNK_WIDTH - 1][1] == 0,
			game->entities->chunks[zMin].tiles[0][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
			game->entities->chunks[yPlus].tiles[0][0][0] == 0,
			tiles[0][CHUNK_WIDTH - 2][0] == 0,
			tiles[1][CHUNK_WIDTH - 1][0] == 0,
			game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][CHUNK_WIDTH - 1][0] == 0);
	if (xMin != -1 && yMin != -1 && zPlus != -1)
		GenTile(data, 0, 0, CHUNK_WIDTH - 1,
			game->entities->chunks[zPlus].tiles[0][0][0] == 0,
			tiles[0][0][CHUNK_WIDTH - 2] == 0,
			tiles[0][1][CHUNK_WIDTH - 1] == 0,
			game->entities->chunks[yMin].tiles[0][CHUNK_WIDTH - 1][CHUNK_WIDTH - 1] == 0,
			tiles[1][0][CHUNK_WIDTH - 1] == 0,
			game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][0][CHUNK_WIDTH - 1] == 0);
	if (xMin != -1 && yMin != -1 && zMin != -1)
		GenTile(data, 0, 0, 0,
			tiles[0][0][1] == 0,
			game->entities->chunks[zMin].tiles[0][0][CHUNK_WIDTH - 1] == 0,
			tiles[0][1][0] == 0,
			game->entities->chunks[yMin].tiles[0][CHUNK_WIDTH - 1][0] == 0,
			tiles[1][0][0] == 0,
			game->entities->chunks[xMin].tiles[CHUNK_WIDTH - 1][0][0] == 0);
#pragma endregion

	vector<uint> indices = vector<uint>(data.size() / 4 * 6);
	for (uint i = 0; i < indices.size(); i++)
		indices[i] = quadInd[i % 6] + i / 6 * 4;


	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), static_cast<void*>(data.data()), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), static_cast<void*>(indices.data()), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

	glEnableVertexAttribArray(2);

	indCount = static_cast<uint>(indices.size());
}

void Game::Start()
{
	cursorUnlockCount = 0;

	tTime2 = 0;

	tutorialState = 0;
	growthSpeedMul = 1;

	specialData.clear();

	srand(static_cast<uint>(time(NULL) % UINT_MAX));

	planet = std::make_unique<Planet>();
	logBook = make_unique<LogBook>();

	entities = make_unique<Entities>();
	unique_ptr<Player> playerUnique = characters[UnEnum(settings.character)]->PClone(settings.startSeeds);
	player = playerUnique.get();

	unique_ptr<Entity> baseUnique = charBases[UnEnum(settings.character)]->Clone(player->pos);
	base = static_cast<Base*>(baseUnique.get());
	player->base = base;

	planet->faction1Spawns->defaultTarget = base;
	planet->wildSpawns->defaultTarget = player;

	for (int i = 0; i < 100; i++)
	{
		if (!entities->OverlapsTile(base->pos, base->radius))
			break;
		else
			base->pos.z++;
	}
	player->pos = base->pos + up * (base->radius + player->radius);

	entities->push_back(std::move(playerUnique));
	entities->push_back(std::move(baseUnique));

	playerAlive = true;
	totalGamePoints = 0;

	waveCount = 0;
	shouldSpawnBoss = false;

	screenShkX.SetFrequency(5.f);
	screenShkX.SetFractalLacunarity(2.0f);
	screenShkX.SetFractalGain(0.5f);
	screenShkX.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
	screenShkX.SetSeed(static_cast<int>(time(NULL)));

	screenShkY = screenShkX;
	screenShkY.SetSeed(static_cast<int>(time(NULL) + 1));

	screenShkZ = screenShkY;
	screenShkZ.SetSeed(static_cast<int>(time(NULL) + 1));

	screenShake = 0.0f;

	startCallbacks[UnEnum(startCallback)]();
}
	
void Game::Update()
{
	inputs.UpdateMouse(window, settings.sensitivity);

	updateModes[UnEnum(updateMode)]();
}

namespace StartCallbacks
{
	void Default()
	{
		game->planet->wildSpawns->waitTime = 15;
		game->planet->faction1Spawns->waitTime = 15;
	}

	void Tutorial1()
	{
		// Disable enemy spawns:
		game->planet->faction1Spawns->speedMul = 0;
		game->planet->wildSpawns->speedMul = 0;
		// State that this was in a tutorial in the death message:
		game->specialData.push_back({ "In Combat Tutorial", RGBA(255, 127) });
	}

	void Tutorial2()
	{
		// Disable enemy spawns:
		game->planet->faction1Spawns->speedMul = 0;
		game->planet->wildSpawns->speedMul = 0;
		// Increase plant growth rates:
		game->growthSpeedMul = 3;
		// Give correct items:
		game->player->items.push_back(Resources::Seeds::shadeShrubSeed.Clone(5));
		// State that this was in a tutorial in the death message:
		game->specialData.push_back({ "In Farming and building Tutorial", RGBA(255, 127) });
	}

	void Tutorial3()
	{
		// Disable enemy spawns:
		game->planet->faction1Spawns->speedMul = 0;
		game->planet->wildSpawns->speedMul = 0;
		// State that this was in a tutorial in the death message:
		game->specialData.push_back({ "In Inventory Management Tutorial", RGBA(255, 127) });
	}
}

namespace UpdateModes
{
	void MainMenu()
	{
		float sineLerp = sinf(glfwGetTime() * 2 * PI_F) * 0.5f + 0.5f;

		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (game->logBook->isOpen)
			return game->logBook->DUpdate();

		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.875f), ScrHeight() / 10.0f, "Character Select",
			RGBA(0, 255, 255).CLerp(RGBA(0, 0, 255), sineLerp), RGBA(0, 127, 127).CLerp(RGBA(0, 0, 127), sineLerp)))
		{
			game->scroll = 0;
			game->updateMode = UPDATEMODE::CHAR_SELECT;
		}
		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.75f), ScrHeight() / 10.0f, "Exit"))
			glfwSetWindowShouldClose(game->window, GL_TRUE);
		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.625f), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::EASY], game->settings.difficulty == DIFFICULTY::EASY ?
			RGBA(0, 255) : RGBA(255, 255, 255), game->settings.difficulty == DIFFICULTY::EASY ? RGBA(0, 127) : RGBA(127, 127, 127)))
		{
			game->settings.difficulty = DIFFICULTY::EASY;
			game->settings.Write();
		}
		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.5f), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::MEDIUM], game->settings.difficulty == DIFFICULTY::MEDIUM ?
			RGBA(255, 255) : RGBA(255, 255, 255), game->settings.difficulty == DIFFICULTY::MEDIUM ? RGBA(127, 127) : RGBA(127, 127, 127)))
		{
			game->settings.difficulty = DIFFICULTY::MEDIUM;
			game->settings.Write();
		}
		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.375f), ScrHeight() / 10.0f, difficultyStrs[DIFFICULTY::HARD], game->settings.difficulty == DIFFICULTY::HARD ?
			RGBA(255) : RGBA(255, 255, 255), game->settings.difficulty == DIFFICULTY::HARD ? RGBA(127) : RGBA(127, 127, 127)))
		{
			game->settings.difficulty = DIFFICULTY::HARD;
			game->settings.Write();
		}
		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.25f), ScrHeight() / 10.0f, "Settings"))
			game->logBook->OpenSettings();
		if (InputHoverSquare(Vec2(0, ScrHeight() * 0.125f), ScrHeight() / 10.0f, "LogBook"))
			game->logBook->OpenLogBook();
		if (InputHoverSquare(vZero, ScrHeight() / 10.0f, "Tutorial Select"))
		{
			game->scroll = 0;
			game->updateMode = UPDATEMODE::TUTORIAL_SELECT;
		}
	}

	void CharSelect()
	{
		float sineLerp = sinf(glfwGetTime() * 2 * PI_F) * 0.5f + 0.5f;

		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.875f)), ScrHeight() / 10.0f, "Seed Select",
			RGBA(0, 255, 255).CLerp(RGBA(0, 0, 255), sineLerp), RGBA(0, 127, 127).CLerp(RGBA(0, 0, 127), sineLerp)))
		{
			game->scroll = 0;
			game->updateMode = UPDATEMODE::SEED_SELECT;
		}
		if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * 0.75f)), ScrHeight() / 10.0f, "Back") || game->inputs.keys[KeyCode::PAUSE].pressed)
			game->updateMode = UPDATEMODE::MAINMENU;

		for (int i = 0; i < characters.size(); i++)
			if (InputHoverSquare(iVec2(0, static_cast<int>(ScrHeight() * (0.625f - i * 0.125f))), ScrHeight() / 10.0f, characters[i]->name,
				UnEnum(game->settings.character) == i ? characters[i]->color : RGBA(255, 255, 255), UnEnum(game->settings.character) == i ? characters[i]->color / 2 : RGBA(127, 127, 127)))
			{
				game->settings.character = static_cast<CHARS>(i); // Doesn't break so that it renders the rest as InputHoverSquare does rendering.
				game->settings.Write();
			}
	}

	void SeedSelect()
	{
		float sineLerp = sinf(glfwGetTime() * 2 * PI_F) * 0.5f + 0.5f;

		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		game->scroll = ClampF(game->scroll - game->inputs.mouseScrollF, 0, max(0, UnEnum(SEEDINDICES::COUNT) - 8));
		game->inputs.mouseScrollF = 0;
		float offset = ScrHeight() * 0.125f * game->scroll;

		if (InputHoverSquare(Vec2(0, offset + ScrHeight() * 0.875f), ScrHeight() / 10.0f, "Begin",
			RGBA(0, 255, 255).CLerp(RGBA(0, 0, 255), sineLerp), RGBA(0, 127, 127).CLerp(RGBA(0, 0, 127), sineLerp)))
		{
			game->startCallback = STARTCALLBACK::NONE;
			game->preUpdate = PREUPDATE::DEFAULT;
			game->postUpdate = POSTUPDATE::DEFAULT;
			game->updateMode = UPDATEMODE::IN_GAME;
			game->Start();
		}
		if (InputHoverSquare(Vec2(0, offset + ScrHeight() * 0.75f), ScrHeight() / 10.0f, "Back") || game->inputs.keys[KeyCode::PAUSE].pressed)
		{
			game->scroll = 0;
			game->updateMode = UPDATEMODE::CHAR_SELECT;
		}

		int selectedTotal = 0;
		for (int i = 0; i < UnEnum(SEEDINDICES::COUNT); i++)
			if (game->settings.startSeeds[i])
				selectedTotal++;
		int allowedTotal = difficultySeedSelectQuantity[game->settings.difficulty];
		if (selectedTotal > allowedTotal)
		{
			for (int i = 0; i < UnEnum(SEEDINDICES::COUNT); i++)
				game->settings.startSeeds[i] = false;
			game->settings.Write();
		}
		string strAmountRemaining = allowedTotal == selectedTotal ? "Complete" : "Choose " + std::to_string(allowedTotal - selectedTotal) + " more";
		font.Render(strAmountRemaining, Vec2(ScrWidth() - ScrHeight() * 0.1f * font.TextWidthTrue(strAmountRemaining),
			ScrHeight() * 0.9f), ScrHeight() * 0.1f, RGBA(0, 0, 255).CLerp(RGBA(255, 127), sineLerp));
		for (int i = 0; i < UnEnum(SEEDINDICES::COUNT); i++)
		{
			if (InputHoverSquare(Vec2(0, offset + ScrHeight() * (0.625f - i * 0.125f)), ScrHeight() / 10.0f, Plants::plants[i]->name,
				game->settings.startSeeds[i] ? Plants::plants[i]->color.CLerp(RGBA(), sineLerp) : RGBA(255, 255, 255), game->settings.startSeeds[i] ? Plants::plants[i]->color / 2 : RGBA(127, 127, 127)) &&
				(selectedTotal < allowedTotal || game->settings.startSeeds[i]))
			{
				game->settings.startSeeds[i] ^= true; // Doesn't break so that it renders the rest as InputHoverSquare does rendering.
				game->settings.Write();
			}
		}
	}

	void TutorialSelect()
	{
		float sineLerp = sinf(glfwGetTime() * 2 * PI_F) * 0.5f + 0.5f;

		currentFramebuffer = 0;
		UseFramebuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		game->scroll = ClampF(game->scroll - game->inputs.mouseScrollF, 0, max(0, static_cast<int>(tutorialStrs.size() - 9)));
		game->inputs.mouseScrollF = 0;
		float offset = ScrHeight() * 0.125f * game->scroll;
		if (InputHoverSquare(Vec2(0, offset + ScrHeight() * 0.875f), ScrHeight() / 10.0f, "Back") || game->inputs.keys[KeyCode::PAUSE].pressed)
		{
			game->scroll = 0;
			game->updateMode = UPDATEMODE::MAINMENU;
		}

		for (int i = 0; i < tutorialStrs.size(); i++)
		{
			if (InputHoverSquare(Vec2(0, offset + ScrHeight() * (0.75f - i * 0.125f)), ScrHeight() * 0.1f, tutorialStrs[i],
				RGBA(255, 255, 255), RGBA(127, 127, 127)))
			{
				game->startCallback = (STARTCALLBACK)(i + 1);
				game->preUpdate = (PREUPDATE)(i + 1);
				game->postUpdate = (POSTUPDATE)(i + 1);
				game->updateMode = UPDATEMODE::IN_GAME;
				for (int j = 0; j < UnEnum(SEEDINDICES::COUNT); j++)
					game->settings.startSeeds[j] = false;
				game->Start();
			}
		}
	}

	void InGame()
	{
		// If the player has died then we should switch to the death screen:
		if (!playerAlive)
		{
			game->updateMode = UPDATEMODE::DEAD;
			return;
		}
		// We're not dead so the update code gets run.
#pragma region Pre-Update
		if (game->dTime >= 1.f)
			return; // Don't run frame if there was a GIGANTIC frame drop.
		// This is normally off.
		glEnable(GL_DEPTH_TEST);
		// Prepare current framebuffer to be used in rendering of the frame.
		currentFramebuffer = MAINSCREEN;
		UseFramebuffer();
		game->inputs.Update(game->window);
		// In TUpdate such that time doesn't progress whilst paused.
		if (game->updateMode == UPDATEMODE::IN_GAME)
		{
			tTime += game->dTime;
			tTime2 += game->dTime;
		}
		else
		{
			game->dTime = 0; // Don't let time pass while paused.
			game->inputs.mouseOffset = vZero; // Don't let the player do any rotation while paused.
		}

		if ((game->lastCursorUnlockCount == 0) != (game->cursorUnlockCount == 0))
		{
			game->lastCursorUnlockCount = game->cursorUnlockCount;
			if (game->cursorUnlockCount != 0)
				game->inputs.mouseOffset = vZero; // If the player just entered something like a menu then ignore the mouse offset.
		}

		if (playerAlive && game->inputs.keys[KeyCode::PAUSE].pressed)
		{
			if (game->updateMode == UPDATEMODE::IN_GAME)
			{
				game->updateMode = UPDATEMODE::PAUSED;
				game->cursorUnlockCount++;
			}
			else
			{
				game->updateMode = UPDATEMODE::IN_GAME;
				game->cursorUnlockCount--;
			}
		}

		game->screenShake *= powf(0.25f, game->dTime);
		game->screenOffset = Vec3(game->screenShkX.GetNoise(tTime, 0.f), game->screenShkY.GetNoise(tTime, 0.f), game->screenShkZ.GetNoise(tTime, 0.f)) * game->screenShake;

		game->brightness = game->planet->GetBrightness();

		preUpdates[UnEnum(game->preUpdate)]();
#pragma endregion
		
#pragma region Update Stuff
		game->entities->Update(); // Updates all entities.
		// Now that the player has moved mark where it is for rendering:
		game->camPos = game->screenOffset + game->player->pos;

		game->camera = glm::lookAt(game->camPos, game->camPos + game->player->camDir, up);
		game->camRot = glm::lookAt(vZero, game->player->camDir, up);
		game->camForward = game->player->camDir;
		game->camRight = game->player->rightDir;
		game->camUp = game->player->upDir;

		game->cameraInv = glm::inverse(game->camera);
		game->perspective = glm::perspective(game->fov, screenRatio, game->nearDist, game->farDist);
		game->CalcFrustum();

		game->lastPlayerPos = game->player->pos;
		glUseProgram(circleShader);
		glUniformMatrix4fv(glGetUniformLocation(circleShader, "camera"), 1, GL_FALSE, glm::value_ptr(game->camera));
		glUniformMatrix4fv(glGetUniformLocation(circleShader, "cameraInv"), 1, GL_FALSE, glm::value_ptr(game->cameraInv));
		glUniformMatrix4fv(glGetUniformLocation(circleShader, "perspective"), 1, GL_FALSE, glm::value_ptr(game->perspective));
		glUniform3f(glGetUniformLocation(circleShader, "camPos"), game->camPos.x, game->camPos.y, game->camPos.z);
		glUseProgram(cylinderShader);
		glUniformMatrix4fv(glGetUniformLocation(cylinderShader, "camera"), 1, GL_FALSE, glm::value_ptr(game->camera));
		glUniformMatrix4fv(glGetUniformLocation(cylinderShader, "camRot"), 1, GL_FALSE, glm::value_ptr(game->camRot));
		glUniformMatrix4fv(glGetUniformLocation(cylinderShader, "cameraInv"), 1, GL_FALSE, glm::value_ptr(game->cameraInv));
		glUniformMatrix4fv(glGetUniformLocation(cylinderShader, "perspective"), 1, GL_FALSE, glm::value_ptr(game->perspective));
		glUniform3f(glGetUniformLocation(cylinderShader, "camPos"), game->camPos.x, game->camPos.y, game->camPos.z);
		glUseProgram(capsuleShader);
		glUniformMatrix4fv(glGetUniformLocation(capsuleShader, "camera"), 1, GL_FALSE, glm::value_ptr(game->camera));
		glUniformMatrix4fv(glGetUniformLocation(capsuleShader, "camRot"), 1, GL_FALSE, glm::value_ptr(game->camRot));
		glUniformMatrix4fv(glGetUniformLocation(capsuleShader, "cameraInv"), 1, GL_FALSE, glm::value_ptr(game->cameraInv));
		glUniformMatrix4fv(glGetUniformLocation(capsuleShader, "perspective"), 1, GL_FALSE, glm::value_ptr(game->perspective));
		glUniform3f(glGetUniformLocation(capsuleShader, "camPos"), game->camPos.x, game->camPos.y, game->camPos.z);
		glUseProgram(coneShader);
		glUniformMatrix4fv(glGetUniformLocation(coneShader, "camera"), 1, GL_FALSE, glm::value_ptr(game->camera));
		glUniformMatrix4fv(glGetUniformLocation(coneShader, "camRot"), 1, GL_FALSE, glm::value_ptr(game->camRot));
		glUniformMatrix4fv(glGetUniformLocation(coneShader, "cameraInv"), 1, GL_FALSE, glm::value_ptr(game->cameraInv));
		glUniformMatrix4fv(glGetUniformLocation(coneShader, "perspective"), 1, GL_FALSE, glm::value_ptr(game->perspective));
		glUniform3f(glGetUniformLocation(coneShader, "camPos"), game->camPos.x, game->camPos.y, game->camPos.z);
		glUseProgram(chunkShader);
		glUniformMatrix4fv(glGetUniformLocation(chunkShader, "camera"), 1, GL_FALSE, glm::value_ptr(game->camera));
		glUniformMatrix4fv(glGetUniformLocation(chunkShader, "camRot"), 1, GL_FALSE, glm::value_ptr(game->camRot));
		glUniformMatrix4fv(glGetUniformLocation(chunkShader, "perspective"), 1, GL_FALSE, glm::value_ptr(game->perspective * game->camera));
		glUseProgram(shadowShader);
		glUniformMatrix4fv(glGetUniformLocation(shadowShader, "perspective"), 1, GL_FALSE, glm::value_ptr(game->perspective * game->camera));
		glUniform2f(glGetUniformLocation(shadowShader, "screenDim"), float(trueScreenWidth), float(trueScreenHeight));

		JRGB skyCol = game->planet->SkyCol();
		glClearColor(skyCol.r / 255.f, skyCol.g / 255.f, skyCol.b / 255.f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		static const float normalDef[] = { 0, 0, 0, 0 };
		glClearBufferfv(GL_COLOR, 1, normalDef);
		static const float noPosition[] = { 0, 0, 0, 0 };
		glClearBufferfv(GL_COLOR, 2, noPosition);
		game->entities->DUpdate(); // Draws all entities.
		game->ApplyLighting(); // Apply lighting.
		glDisable(GL_DEPTH_TEST);
		game->DrawFramebufferOnto(TRUESCREEN);

		game->entities->UIUpdate(); // Draws UI of uiactive entities.
#pragma endregion

#pragma region Post Update
		if (game->inputs.keys[KeyCode::HIDEUI].pressed)
			game->showUI = !game->showUI;
		if (game->showUI && playerAlive)
		{
			if (game->shouldSpawnBoss)
			{
				float timeTillNextBoss = 60.0f - tTime + game->timeStartBossPrep;
				font.Render(std::to_string(waveCount) + ":" + ToStringWithPrecision(ModF(tTime2, 60.f), 1) + " " +
					std::to_string(int(timeTillNextBoss)) + "." +
					std::to_string(int(timeTillNextBoss * 10) - int(timeTillNextBoss) * 10), iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.95f)), ScrHeight() / 20.0f,
					game->planet->faction1Spawns->superWave ? RGBA(255, 255) : RGBA(0, 255, 255));
			}
			else
				font.Render(std::to_string(waveCount) + ":" + ToStringWithPrecision(ModF(tTime2, 60.f), 1),
					iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.95f)), ScrHeight() / 20.0f, game->planet->faction1Spawns->superWave ? RGBA(255, 255) : RGBA(0, 255, 255));
			font.Render(std::to_string(game->player->health), iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.9f)), ScrHeight() / 20.0f, RGBA(63));
			font.Render(to_string(totalGamePoints), iVec2(-ScrWidth(), static_cast<int>(ScrHeight() * 0.85f)), ScrHeight() / 20.0f, RGBA(63, 63));
		}

		postUpdates[UnEnum(game->postUpdate)]();

		frameCount++;
#pragma endregion
	}

	void Paused()
	{
		updateModes[UnEnum(UPDATEMODE::IN_GAME)]();

		if (!playerAlive) return;

		currentFramebuffer = TRUESCREEN;
		UseFramebuffer();
		game->inputs.UpdateKey(game->window, game->inputs.keys[KeyCode::PAUSE]);

		if (game->logBook->isOpen)
			game->logBook->DUpdate();
		else // Don't render this stuff whenever the logbook's open!
		{
			if (InputHoverSquare(Vec2(0, ScrHeight() * 0.875f), ScrHeight() / 10.0f, "Return"))
			{
				game->cursorUnlockCount--;
				game->updateMode = UPDATEMODE::IN_GAME;
			}
			if (InputHoverSquare(Vec2(0, ScrHeight() * 0.75f), ScrHeight() / 10.0f, "Restart"))
			{
				game->Start();
				game->updateMode = UPDATEMODE::IN_GAME;
			}
			if (InputHoverSquare(Vec2(0, ScrHeight() * 0.625f), ScrHeight() / 10.0f, "Main Menu"))
				game->updateMode = UPDATEMODE::MAINMENU;
			if (InputHoverSquare(Vec2(0, ScrHeight() * 0.5f), ScrHeight() / 10.0f, "Settings"))
				game->logBook->OpenSettings();
			if (InputHoverSquare(Vec2(0, ScrHeight() * 0.375f), ScrHeight() / 10.0f, "Log Book"))
				game->logBook->OpenLogBook();
		}
	}

	void Dead()
	{
		if (currentFramebuffer != 0)
		{
			currentFramebuffer = 0;
			UseFramebuffer();
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		vector<std::pair<string, RGBA>> startSeedData;
		for (int j = 0; j < Resources::Seeds::plantSeeds.size(); j++)
			if (game->settings.startSeeds[j])
				startSeedData.push_back({ Resources::Seeds::plantSeeds[j]->name, Resources::Seeds::plantSeeds[j]->color });

		float totalLines = 9 + startSeedData.size() + game->specialData.size() + 0.2f;
		float scale = float(ScrHeight()) / totalLines, scale2 = scale * 0.8f, scale3 = scale * 2, scale4 = scale2 * 2;
		float offset = scale * 0.2f, offset2 = offset * 2;
		int i = 0;
	
		font.Render(difficultyStrs[game->settings.difficulty], { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4,
			{ game->settings.difficulty == DIFFICULTY::EASY ? 0u : 255u, game->settings.difficulty == DIFFICULTY::HARD ? 0u : 255u });

		for (std::pair<string, RGBA> p : game->specialData)
			font.Render(p.first, { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4, p.second);
	
		if (InputHoverSquare(Vec2(0, offset + scale * i++), scale2, "Main menu"))
			game->updateMode = UPDATEMODE::MAINMENU;
		if (InputHoverSquare(iVec2(0, offset + scale * i++), scale2, "Restart"))
		{
			game->Start();
			game->updateMode = UPDATEMODE::IN_GAME;
		}

		font.Render(deathCauseName, { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4, { 255, 255, 255 });
		font.Render(deathName + " was killed by :", { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4, { 255, 255, 255 });
		font.Render(to_string(totalGamePoints) + " points", { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4, { 255, 255, 255 });
		font.Render("Lasted " + std::to_string(waveCount) + ":" + ToStringWithPrecision(ModF(tTime2, 60.f), 1), { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4, { 255, 255, 255 });

		for (std::pair<string, RGBA> p : startSeedData)
			font.Render(p.first, { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4, p.second);
		font.Render(startSeedData.size() ? "Started with:" : "Started seedless", { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4, { 255, 255, 255 });
		font.Render("Started as " + characters[UnEnum(game->settings.character)]->name, { -ScrWidth(), -ScrHeight() + offset2 + scale3 * i++ }, scale4, characters[UnEnum(game->settings.character)]->color);
	}
}

namespace PreUpdates
{
	void Default()
	{
		// Spawn boss:
		if (game->inputs.keys[KeyCode::ENTER].pressed && !game->shouldSpawnBoss)
		{
			game->shouldSpawnBoss = true;
			game->timeStartBossPrep = tTime;
		}

		if (game->shouldSpawnBoss && tTime - game->timeStartBossPrep >= 60.0f)
		{
			game->planet->bosses->SpawnOneRandom();
			game->shouldSpawnBoss = false;
		}
		waveCount = tTime2 / 60;

		// Enemy spawn stuff:
		game->planet->faction1Spawns->Update();
		game->planet->wildSpawns->Update();
	}

	void Tutorial1()
	{
		Default();

		if (game->tutorialState == 0 && game->inputs.MoveDir() != vZero)
			game->tutorialState = 1;
		if (game->tutorialState == 1 && game->player->lastPrimary != -game->player->primaryTime)
			game->tutorialState = 2;
		if (game->tutorialState == 2 && game->player->lastUtility != -game->player->utilityTime)
			game->tutorialState = 3;
		if (game->tutorialState == 3 && game->player->lastSecondary != -game->player->secondaryTime)
		{
			game->tutorialState = 4;
			// Make enemies spawn:
			game->planet->faction1Spawns->speedMul = 3;
			game->planet->wildSpawns->speedMul = 3;
		}
		if (game->tutorialState >= 4 && game->tutorialState < 9 && floorf(tTime) != floorf(tTime - game->dTime))
			game->tutorialState++;
		if (game->tutorialState >= 9 && game->tutorialState < 14 && floorf(tTime) != floorf(tTime - game->dTime))
			game->tutorialState++;
	}

	void Tutorial2()
	{
		Default();

		if (game->tutorialState == 0)
			for (Entity* entity : game->entities->sortedNCEntities)
				if (entity->baseClass == &Plants::Shrubs::shadeShrub)
				{
					game->tutorialState++;
					game->player->items.push_back(Resources::Seeds::cheeseVineSeed.Clone(5));
				}
		if (game->tutorialState == 1)
			for (Entity* entity : game->entities->sortedNCEntities)
				if (entity->baseClass == &Plants::Vines::cheeseVine)
					game->tutorialState++;
		if (game->tutorialState == 2 && game->player->items.CanMake(Defences::Towers::pulseTurret.recipe))
			game->tutorialState++;
		if (game->tutorialState >= 3 && game->tutorialState < 33 && floorf(tTime) != floorf(tTime - game->dTime))
			game->tutorialState++;
		if (game->tutorialState >= 33 && game->tutorialState < 38 && floorf(tTime) != floorf(tTime - game->dTime))
		{
			game->tutorialState++;
			// Make enemies spawn:
			game->planet->faction1Spawns->speedMul = 6;
			game->planet->wildSpawns->speedMul = 6;
		}
	}

	void Tutorial3()
	{
		Default();

		if (game->tutorialState == 0 && game->player->items.isOpen)
			game->tutorialState++;
		if (game->tutorialState == 1 && game->player->items[game->player->items.size() - 1].count > 0)
			game->tutorialState++;
	}
}

namespace PostUpdates
{
	void Default() { }

	void Tutorial1()
	{
		Default();
		vector<string> tutLines;
		switch (game->tutorialState)
		{
		case 0:
			tutLines = { "WASD to move", "Mouse to look around" };
			break;
		case 1:
			tutLines = { "Left click to shoot", "You shoot whatever is selected", "More info in inventory management" };
			break;
		case 2:
			tutLines = { "Shift to activate utility", "Many characters require you to hold a direction", "Each character has a different utility"};
			break;
		case 3:
			tutLines = { "R to activate secondary", "Each character has a different secondary"};
			break;
		default:
			if (game->tutorialState < 9)
				tutLines = { "Enemies spawn in waves", "Most enemies belong to factions", "Enemies of seperate factions will fight" };
			else if (game->tutorialState < 14)
				tutLines = { "Good luck!" };
			break;
		}
		for (int i = 0; i < tutLines.size(); i++)
			font.Render(tutLines[i], Vec2(ScrWidth() - ScrHeight() * 0.1f * font.TextWidthTrue(tutLines[i]),
				ScrHeight() * (0.9f - i * 0.1f)), ScrHeight() * 0.1f, RGBA(127, 127, 127));
	}

	void Tutorial2()
	{
		Default();
		vector<string> tutLines;
		switch (game->tutorialState)
		{
		case 0:
			tutLines = { "You will start a majority of runs with seeds", "Seeds can be planted by shooting them at the ground",
				"Seeds come in many varieties", "Plants can grow on brown soil", "More on soil in advanced farming",
				"Plant growth speed has been tripled for this tutorial", "Plant a shade seed to proceed" };
			break;
		case 1:
			tutLines = { "Shrubs grow outwards and are fairly simple", "Vines grow whatever direction they are pointed",
				"Plant a cheese vine to proceed"};
			break;
		case 2:
			tutLines = { "25 shade + 25 cheese = a pulse turret", "Gather these resources to proceed", "Plants only grow when they are in light",
				"Cheese is a light source", "10 cheese = lantern", "Lanterns are more powerful cheese"};
			break;
		default:
			if (game->tutorialState < 33)
				tutLines = { "Press 'V' to access the build menu", "In the build menu left click creates and right click destroys",
				"Right clicking plants destroys them, do this to dead plants", "It would be wise to create pulse turrets to defend",
				"Turrets normally attack nearby enemies", "More on farming will be found in advanced farming" };
			else if (game->tutorialState < 38)
				tutLines = { "Have fun!" };
			break;
		}
		for (int i = 0; i < tutLines.size(); i++)
			font.Render(tutLines[i], Vec2(ScrWidth() - ScrHeight() * 0.1f * font.TextWidthTrue(tutLines[i]),
				ScrHeight() * (0.9f - i * 0.1f)), ScrHeight() * 0.1f, RGBA(127, 127, 127));
	}

	void Tutorial3()
	{
		Default();
		vector<string> tutLines;
		switch (game->tutorialState)
		{
		case 0:
			tutLines = { "Press 'tab' to open your inventory" };
			break;
		case 1:
			tutLines = { "You can click 2 items to swap them", "The leftmost slots are your hotbar", "The bottom right slot is the offhand",
				"The offhand is shot by right click", "Put something in your offhand to proceed"};
			break;
		case 2:
			tutLines = { "I'm not sure quite what else to put in this tutorial. =\\"};
			break;
		case 3:
			tutLines = { "R to activate secondary", "Each character has a different secondary" };
			break;
		default:
			if (game->tutorialState < 9)
				tutLines = { "Enemies spawn in waves", "Most enemies belong to factions", "Enemies of seperate factions will fight" };
			else if (game->tutorialState < 14)
				tutLines = { "Good luck!" };
			break;
		}
		for (int i = 0; i < tutLines.size(); i++)
			font.Render(tutLines[i], Vec2(ScrWidth() - ScrHeight() * 0.1f * font.TextWidthTrue(tutLines[i]),
				ScrHeight() * (0.9f - i * 0.1f)), ScrHeight() * 0.1f, RGBA(127, 127, 127));
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
	Vec3 sunDir = Vec3(camRot * glm::vec4(planet->sunDir, 1));
	glUniform3f(glGetUniformLocation(sunShader, "direction"), sunDir.x, sunDir.y, sunDir.z);
	glUniform1i(glGetUniformLocation(sunShader, "colorBand"), int(settings.colorBand));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainScreen->normalBuffer);
	glUniform1i(glGetUniformLocation(sunShader, "normalMap"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mainScreen->positionBuffer);
	glUniform1i(glGetUniformLocation(sunShader, "positionMap"), 1);
	screenSpaceQuad.Draw();


	glUseProgram(shadowShader);
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

void Game::MenuedEntityDied(Entity* entity)
{
	if (player->currentMenuedEntity == entity)
		player->currentMenuedEntity = nullptr;
}

bool Game::End()
{
	if (endTimer == -1)
	{
		endTimer = glfwGetTime();
		glfwGetWindowSize(window, &endWidth, &endHeight);
		glfwSetWindowTitle(window, "Martionotany - Goodbye!");
		if (IsFullscreen())
			glfwSetWindowMonitor(window, nullptr, 100, 100, START_SCR_WIDTH, START_SCR_HEIGHT, 0);
	}
	else
	{
		float mul = 1 - glfwGetTime() + endTimer;
		if (mul <= 0) return true;
		glfwSwapInterval(int(settings.vSync));
		glClearColor(0, 0, 0, mul);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSetWindowSize(window, endWidth * mul, endHeight * mul);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return false;
}

void Inputs::Update(GLFWwindow* window)
{
	UpdateKey(window, keys[KeyCode::UP]);
	UpdateKey(window, keys[KeyCode::LEFT]);
	UpdateKey(window, keys[KeyCode::DOWN]);
	UpdateKey(window, keys[KeyCode::RIGHT]);
	UpdateKey(window, keys[KeyCode::JUMP]);

	// Primary and offhand are bound to mouse buttons and must be handled seperately.
	UpdateKey(window, keys[KeyCode::SECONDARY]);
	UpdateKey(window, keys[KeyCode::UTILITY]);

	UpdateKey(window, keys[KeyCode::BUILD]);
	UpdateKey(window, keys[KeyCode::CROUCH]);
	UpdateKey(window, keys[KeyCode::INVENTORY]);

	UpdateKey(window, keys[KeyCode::ENTER]);
	UpdateKey(window, keys[KeyCode::HIDEUI]);
	if (game->settings.canChangeRow)
	{
		UpdateKey(window, keys[KeyCode::ROW_LEFT]);
		UpdateKey(window, keys[KeyCode::ROW_RIGHT]);
	}
	UpdateKey(window, keys[KeyCode::PAUSE]);

	UpdateKey(window, keys[KeyCode::COMMA]);
	UpdateKey(window, keys[KeyCode::PERIOD]);
	UpdateKey(window, keys[KeyCode::SLASH]);
	UpdateKey(window, keys[KeyCode::PHASE]);
}