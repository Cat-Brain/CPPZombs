#include "Player.h"

class Planet
{
public:
	float dawnTime, dayTime, duskTime, nightTime, ambientLight, ambientDark;
	Enemies enemies, bosses;
	JRGB backgroundBaseColor, backgroundColorWidth;
	FastNoiseLite backgroundNoise;

	Planet()
	{
		dawnTime = RandFloat() * 59.99f + 0.01f; // Can't be 0 or it will crash.
		dayTime = RandFloat() * 60.0f;
		duskTime = RandFloat() * 60.0f;
		nightTime = RandFloat() * 60.0f;

		ambientDark = RandFloat() * 0.5f;
		ambientLight = RandFloat() * 0.5f + 0.5f;


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
	}

	Color GetBackgroundNoise(Vec2f noisePos)
	{
		float randomNoiseValue = backgroundNoise.GetNoise(noisePos.x, noisePos.y);
		return Color(Clamp(backgroundBaseColor.r + (int)roundf(randomNoiseValue * backgroundColorWidth.r) * 5, 0, 255),
			Clamp(backgroundBaseColor.g + (int)roundf(randomNoiseValue * backgroundColorWidth.g) * 5, 0, 255),
			Clamp(backgroundBaseColor.b + (int)roundf(randomNoiseValue * backgroundColorWidth.b) * 5, 0, 255));
	}

	float GetBrightness()
	{
		float wrappedTime = fmodf(tTime, dawnTime + dayTime + duskTime + nightTime);
		float ambientDiff = ambientLight - ambientDark;
		return ambientDark + ambientDiff * ClampF01(min(wrappedTime / dawnTime, (dawnTime + dayTime + duskTime - wrappedTime) / duskTime));
	}
};