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

	bool ApplyLightToPos(JRGB currentColor, int index, Vec2 newPos, Vec2 camMin, Vec2 camMax) // newPos is in relative coordinates.
	{
		return newPos.x >= camMin.x && camMax.x >= newPos.x &&
			newPos.y >= camMin.y && camMax.y >= newPos.y &&
			colorMap[index].MaxEq(currentColor);
	}

	void ApplyLight()
	{
		Vec2 camMin = playerPos - screenDimH * 3, camMax = playerPos + screenDimH * 3, relCamMax = camMax - camMin;
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
			if (ApplyLightToPos(color, index + 1, pos + up, camMin, camMax))
				shouldCheckNextPoses.push_back(pos + up);

			if (ApplyLightToPos(color, index - colorMapWidth, pos + left, camMin, camMax))
				shouldCheckNextPoses.push_back(pos + left);

			if (ApplyLightToPos(color, index - 1, pos + down, camMin, camMax))
				shouldCheckNextPoses.push_back(pos + down);

			if (ApplyLightToPos(color, index + colorMapWidth, pos + right, camMin, camMax))
				shouldCheckNextPoses.push_back(pos + right);

			vector<Vec2> nextShouldCheckPoses(0);
			colorMap[(pos.x - lightMin.x) * colorMapWidth + pos.y - lightMin.y].MaxEq(color);
			for (int i = 0; i < maxDistance - 1 && shouldCheckNextPoses.size(); i++)
			{
				for (Vec2 position : shouldCheckNextPoses)
				{
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
					if (ApplyLightToPos(currentColor, index + 1, position + up, camMin, camMax))
						nextShouldCheckPoses.push_back(position + up);

					if (ApplyLightToPos(currentColor, index - colorMapWidth, position + left, camMin, camMax))
						nextShouldCheckPoses.push_back(position + left);

					if (ApplyLightToPos(currentColor, index - 1, position + down, camMin, camMax))
						nextShouldCheckPoses.push_back(position + down);

					if (ApplyLightToPos(currentColor, index + colorMapWidth, position + right, camMin, camMax))
						nextShouldCheckPoses.push_back(position + right);
				}
				copy(nextShouldCheckPoses.begin(), nextShouldCheckPoses.end(), back_inserter(shouldCheckNextPoses));
				nextShouldCheckPoses.clear();
			}
			for (int x = 0; x < colorMapWidth; x++)
				for (int y = 0; y < colorMapWidth; y++)
				{
					game->shadowMap[x + lightMin.x - camMin.x][y + lightMin.y - camMin.y] += colorMap[x * colorMapWidth + y];
				}
		}
	}
};