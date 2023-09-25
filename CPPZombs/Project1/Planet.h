#include "PsuedoVirtuals.h"

namespace Enemies
{
	class Instance;
	class OvertimeInstance;
}

class Planet
{
public:
	float friction, gravity, dawnTime, dayTime, duskTime, nightTime, ambientLight, ambientDark;
	unique_ptr<Enemies::Instance> bosses;
	unique_ptr<Enemies::OvertimeInstance> faction1Spawns, wildSpawns;
	FastNoiseLite worldNoise, caveNoise;
	Vec3 sunDir;
	JRGB color1, color2, fog;

	Planet();

	float GetBrightness()
	{
		float wrappedTime = fmodf(tTime2, dawnTime + dayTime + duskTime + nightTime);
		float ambientDiff = ambientLight - ambientDark;
		return ambientDark + ambientDiff * ClampF01(min(wrappedTime / dawnTime, (dawnTime + dayTime + duskTime - wrappedTime) / duskTime));
	}

	JRGB GetAmbient(float brightness)
	{
		brightness *= 255;
		byte brightnessB = static_cast<byte>(brightness);
		return JRGB(brightnessB, brightnessB, brightnessB);
	}

	JRGB SkyCol()
	{
		float brightness = GetBrightness();
		return JRGB(static_cast<byte>(brightness * 122), static_cast<byte>(brightness * 250), static_cast<byte>(brightness * 255));
	}
};