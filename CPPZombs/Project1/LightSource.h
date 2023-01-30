#include "Entities.h"

class LightSource
{
protected:
	JRGB lastColor;
	byte lastFalloff = 0;
public:
	Vec2 pos;
	JRGB color;
	byte falloff;
	JRGB* colorMap = nullptr;
	int colorMapWidth = 0;

	LightSource(Vec2 pos = Vec2(0, 0), JRGB color = JRGB(255, 255, 255), byte falloff = 50) :
		pos(pos), color(color), falloff(falloff) { }

	float Range()
	{
		return ceilf(max(color.r, max(color.g, color.b)) / (float)falloff);
	}

	void ApplyLight()
	{
		/*Vec2 camMin = playerPos - ScrDim() * 1.5f, camMax = playerPos + screenDimH * 3, relCamMax = camMax - camMin;
		int maxDistance = Range();
		Vec2 lightMin = pos - vOne * maxDistance, lightMax = pos + vOne * maxDistance;
		colorMapWidth = lightMax.x - lightMin.x + 1;
		if (lastColor != color || lastFalloff != falloff)
		{
			lastColor = color;
			lastFalloff = falloff;
			if (colorMap != nullptr)
				delete colorMap;
			colorMap = new JRGB[colorMapWidth * colorMapWidth];
		}
		for (int i = 0; i < colorMapWidth * colorMapWidth; i++)
			colorMap[i] = JRGB();

		if (pos.x >= camMin.x && camMax.x >= pos.x &&
			pos.y >= camMin.y && camMax.y >= pos.y)
		{
			vector<Vec2> shouldCheckNextPoses{};

			int index = (pos.x - lightMin.x) * colorMapWidth + pos.y - lightMin.y;
			if (colorMap[index + 1].MaxEq(color))
				shouldCheckNextPoses.push_back(pos + up);

			if (colorMap[index - colorMapWidth].MaxEq(color))
				shouldCheckNextPoses.push_back(pos + left);

			if (colorMap[index - 1].MaxEq(color))
				shouldCheckNextPoses.push_back(pos + down);

			if (colorMap[index + colorMapWidth].MaxEq(color))
				shouldCheckNextPoses.push_back(pos + right);

			colorMap[(pos.x - lightMin.x) * colorMapWidth + pos.y - lightMin.y].MaxEq(color);
			for (int i = 0; i < shouldCheckNextPoses.size(); i++)
			{
				Vec2 position = shouldCheckNextPoses[i];
				if (position.x <= 0 || position.x + 1 >= MAP_WIDTH_TRUE || position.y <= 0 || position.y + 1 >= MAP_WIDTH_TRUE)
					continue;
				JRGB currentColor = colorMap[(position.x - lightMin.x) * colorMapWidth + position.y - lightMin.y] - falloff;
				index = game->entities->ChunkAtPos(position)->IndexOfPosition(position);
				if (index != -1)
				{
					JRGB newColor = JRGB((*game->entities)[index]->subsurfaceResistance);
					currentColor -= newColor / 4;
					currentColor.MinEq(newColor);
				}

				index = (position.x - lightMin.x) * colorMapWidth + position.y - lightMin.y;
				if (colorMap[index + 1].MaxEq(currentColor))
					shouldCheckNextPoses.push_back(position + up);

				if (colorMap[index - colorMapWidth].MaxEq(currentColor))
					shouldCheckNextPoses.push_back(position + left);

				if (colorMap[index - 1].MaxEq(currentColor))
					shouldCheckNextPoses.push_back(position + down);

				if (colorMap[index + colorMapWidth].MaxEq(currentColor))
					shouldCheckNextPoses.push_back(position + right);
			}
			for (int x = max(0, camMin.x - lightMin.x); x < min(colorMapWidth, camMax.x - lightMin.x); x++)
				for (int y = max(0, camMin.y - lightMin.y); y < min(colorMapWidth, camMax.y - lightMin.y); y++)
				{
					game->shadowMap[x + lightMin.x - camMin.x][y + lightMin.y - camMin.y] += colorMap[x * colorMapWidth + y];
				}
		}*/
	}
};