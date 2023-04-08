#include "Collectible.h"

class LightSource
{
protected:
public:
	Vec3 pos;
	JRGB color;
	float range;

	LightSource(Vec3 pos = vZero, JRGB color = JRGB(255, 255, 255), float range = 15.0f) :
		pos(pos), color(color), range(range) { }
};