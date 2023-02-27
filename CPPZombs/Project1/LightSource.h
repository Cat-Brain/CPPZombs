#include "Collectible.h"

class LightSource
{
protected:
public:
	Vec2 pos;
	JRGB color;
	float range;

	LightSource(Vec2 pos = Vec2(0, 0), JRGB color = JRGB(255, 255, 255), float range = 15.0f) :
		pos(pos), color(color), range(range) { }
};