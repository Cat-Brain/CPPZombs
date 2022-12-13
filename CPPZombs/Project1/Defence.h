#include "BuildingBlocks.h"

class CollectibleTree : public FunctionalBlock
{
public:
	Collectible* collectible, *seed;
	Color deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;

	CollectibleTree(Collectible* collectible, Collectible* seed, int cyclesToGrow, int deadStage, int chanceForSeed,
		float timePer, Vec2 pos = Vec2(0, 0), Color color = olc::WHITE, Color deadColor = olc::BLACK,
		int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(seed), cyclesToGrow(cyclesToGrow), deadStage(deadStage),
		currentLifespan(0), chanceForSeed(chanceForSeed), deadColor(deadColor),
		FunctionalBlock(timePer, (float)rand() / RAND_MAX * timePer,
			pos, color, mass, maxHealth, health, name)
	{
		Start();
	}

	CollectibleTree(CollectibleTree* baseClass, Vec2 dir, Vec2 pos) :
		CollectibleTree(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return new CollectibleTree(this, dir, pos);
	}

	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime)
	{
		if (currentLifespan < cyclesToGrow)
			screen->Draw(ToRSpace(pos) + Vec2(1, 1), color);
		else if(currentLifespan < deadStage)
		{
			Vec2 rSpacePos = ToRSpace(pos);
			//						\/Up is down\/
			screen->DrawLine(rSpacePos + up, rSpacePos + Vec2(2, 1), color);
			screen->DrawLine(rSpacePos + right, rSpacePos + Vec2(1, 2), color);
		}
		else
		{
			Vec2 rSpacePos = ToRSpace(pos);
			//						\/Up is down\/
			screen->DrawLine(rSpacePos + up, rSpacePos + Vec2(2, 1), deadColor);
			screen->DrawLine(rSpacePos + right, rSpacePos + Vec2(1, 2), deadColor);
		}
	}

	void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs, float dTime)
	{
		if (currentLifespan >= cyclesToGrow && currentLifespan < deadStage)
		{
			if (rand() % 100 < chanceForSeed)
				entities->push_back(seed->Clone(ToCSpace(pos) + Vec2((rand() % 2) * 2, (rand() % 2) * 2)));
			else
				entities->push_back(collectible->Clone(ToCSpace(pos) + Vec2((rand() % 2) * 2, (rand() % 2) * 2)));
		}
		currentLifespan++;
	}

	void UIUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
	{
		Vec2 topLeft = ToRSpace(pos) + Vec2(3, 0);
		if (currentLifespan < cyclesToGrow)
		{
			DrawUIBox(screen, topLeft, topLeft + Vec2(40 + name.length() * 8, 15), "Baby " + name, color, deadColor, collectible->color);
			screen->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 7), ToStringWithPrecision(
				timePer * cyclesToGrow - (tTime - lastTime + timePer * currentLifespan), 1), color);
		}
		else if (currentLifespan < deadStage)
		{
			DrawUIBox(screen, topLeft, topLeft + Vec2(48 + name.length() * 8, 22), "Adult " + name, color, deadColor, collectible->color);
			screen->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 7), ToStringWithPrecision(timePer - tTime + lastTime, 1), color);
			screen->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 14), ToStringWithPrecision(
				timePer * deadStage - (tTime - lastTime + timePer * currentLifespan), 1), color);
		}
		else
			DrawUIBox(screen, topLeft, topLeft + Vec2(40 + name.length() * 8, 8), "Dead " + name, deadColor, color, collectible->color);
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 topLeft = ToRSpace(pos) + Vec2(3, 0), bottomRight;
		if (currentLifespan < cyclesToGrow)
		{
			bottomRight = topLeft + Vec2(40 + name.length() * 8, 15);
		}
		else if (currentLifespan < deadStage)
		{
			bottomRight = topLeft + Vec2(48 + name.length() * 8, 22);
		}
		else
		{
			bottomRight = topLeft + Vec2(40 + name.length() * 8, 8);
		}
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}
};




#pragma region Trees

namespace Resources
{
	PlacedOnLanding* cheese = new PlacedOnLanding(Shootables::cheeseBlock, "Cheese", Color(235, 178, 56), 0);
}

namespace Collectibles
{
	Collectible* cheese = new Collectible(*Resources::cheese);
}

Color copperTreeColor = Color(163, 78, 8), deadCopperTreeColor = Color(94, 52, 17);
CollectibleTree* copperTree = new CollectibleTree(Collectibles::copper, nullptr, 5, 50, 25, 4.0f, vZero, copperTreeColor, deadCopperTreeColor, 1, 1, 1, "Copper tree");
PlacedOnLanding* copperTreeSeed = new PlacedOnLanding(copperTree, "Copper seed", copperTreeColor, 0);
Collectible* cCopperTreeSeed = new Collectible(*copperTreeSeed, vZero);

Color ironTreeColor = Color(67, 90, 99), deadIronTreeColor = Color(45, 47, 48);
CollectibleTree* ironTree = new CollectibleTree(Collectibles::iron, nullptr, 10, 500, 10, 8.0f, vZero, ironTreeColor, deadIronTreeColor, 1, 1, 1, "Iron tree");
PlacedOnLanding* ironTreeSeed = new PlacedOnLanding(ironTree, "Iron tree seed", ironTreeColor, 0);
Collectible* cIronTreeSeed = new Collectible(*ironTreeSeed, vZero);

Color cheeseTreeColor = Color(200, 160, 75), deadCheeseTreeColor = Color(140, 110, 50);
CollectibleTree* cheeseTree = new CollectibleTree(Collectibles::cheese, nullptr, 5, 25, 10, 2.0f, vZero, cheeseTreeColor, deadCheeseTreeColor, 1, 1, 1, "Cheese tree");
PlacedOnLanding* cheeseTreeSeed = new PlacedOnLanding(cheeseTree, "Cheese tree seed", cheeseTreeColor, 0);
Collectible* cCheeseTreeSeed = new Collectible(*cheeseTreeSeed);

#pragma endregion