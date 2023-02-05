#include "Player.h"

class Planet
{
public:
	float dawnTime, dayTime, duskTime, nightTime, ambientLight, ambientDark;
	Enemies::Instance enemies, bosses;
	JRGB color1, color2;
	FastNoiseLite backgroundNoise;

	Planet()
	{
		dawnTime = RandFloat() * 59.99f + 0.01f; // Can't be 0 or it will crash.
		dayTime = RandFloat() * 60.0f;
		duskTime = RandFloat() * 60.0f;
		nightTime = RandFloat() * 60.0f;

		ambientDark = RandFloat() * 0.5f;
		ambientLight = RandFloat() * 0.5f + 0.5f;


		color1.r = rand() % 128 + 64;
		color1.g = rand() % 128 + 64;
		color1.b = rand() % 128 + 64;

		color2.r = rand() % 64 + 128;
		color2.g = rand() % 64 + 128;
		color2.b = rand() % 64 + 128;

		backgroundNoise.SetFrequency(0.06125f);
		backgroundNoise.SetFractalLacunarity(2.0f);
		backgroundNoise.SetFractalGain(0.5f);
		backgroundNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
		backgroundNoise.SetSeed(static_cast<int>(time(NULL)));

		enemies = Enemies::naturalSpawns.RandomClone();
		bosses = Enemies::spawnableBosses.RandomClone();
	}

	float GetBrightness()
	{
		return 0.5;
		float wrappedTime = fmodf(tTime, dawnTime + dayTime + duskTime + nightTime);
		float ambientDiff = ambientLight - ambientDark;
		return ambientDark + ambientDiff * ClampF01(min(wrappedTime / dawnTime, (dawnTime + dayTime + duskTime - wrappedTime) / duskTime));
	}
};