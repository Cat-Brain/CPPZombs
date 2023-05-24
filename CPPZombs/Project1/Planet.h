#include "Game1.h"

namespace Enemies
{
	class Instance;
	class OvertimeInstance;
}

class Planet
{
public:
	float friction, gravity, dawnTime, dayTime, duskTime, nightTime, ambientLight, ambientDark;
	unique_ptr<Enemies::Instance> faction1Spawns, bosses;
	unique_ptr<Enemies::OvertimeInstance> wildSpawns;
	FastNoiseLite worldNoise, caveNoise;
	Vec3 sunDir;
	JRGB color1, color2, fog;

	Planet();

	float GetBrightness()
	{
		float wrappedTime = fmodf(tTime, dawnTime + dayTime + duskTime + nightTime);
		float ambientDiff = ambientLight - ambientDark;
		return ambientDark + ambientDiff * ClampF01(min(wrappedTime / dawnTime, (dawnTime + dayTime + duskTime - wrappedTime) / duskTime));
	}

	JRGB GetAmbient(float brightness)
	{
		return JRGB(1, 1, 1) * static_cast<int>(brightness * 255);
	}
};