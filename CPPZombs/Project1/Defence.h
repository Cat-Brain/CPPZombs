#include "BuildingBlocks.h"

class PlacedOnLanding : public Item
{
public:
	Entity* entityToPlace;

	PlacedOnLanding(Entity* entityToPlace, string name = "NULL", Color color = olc::MAGENTA, int damage = 1, int count = 1):
		Item(name, color, damage, count), entityToPlace(entityToPlace) { }

	PlacedOnLanding(PlacedOnLanding* baseClass, Entity* entityToPlace, string name = "NULL", Color color = olc::MAGENTA, int damage = 1, int count = 1) :
		Item(baseClass, name, color, damage, count), entityToPlace(entityToPlace) { }

	Item Clone(int count) override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, color, damage, count);
	}

	Item Clone() override
	{
		return PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, color, damage, count);
	}

	Item* Clone2(int count) override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, color, damage, count);
	}

	Item* Clone2() override
	{
		return new PlacedOnLanding((PlacedOnLanding*)baseClass, entityToPlace, name, color, damage, count);
	}

	void OnDeath(vector<void*>* collectibles, vector<void*>* entities, Vec2 pos) override
	{
		((Entities*)entities)->push_back(entityToPlace->Clone(pos));
	}
};

class CollectibleTree : public OffsettedFunctionalBlock
{
public:
	Collectible* collectible, *seed;
	Color deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;

	CollectibleTree(Collectible* collectible, Collectible* seed, int cyclesToGrow, int deadStage, int chanceForSeed, int tickPer, Vec2 pos = Vec2(0, 0), Color color = olc::WHITE, Color deadColor = olc::BLACK, Recipe cost = Recipes::dRecipe, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(seed), cyclesToGrow(cyclesToGrow), deadStage(deadStage), currentLifespan(0), chanceForSeed(chanceForSeed), deadColor(deadColor), OffsettedFunctionalBlock(tickPer, pos, color, cost, mass, maxHealth, health, name)
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

	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs)
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

	void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs)
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
};

#pragma region Trees

Color copperTreeColor = Color(163, 78, 8), deadCopperTreeColor = Color(94, 52, 17);
CollectibleTree* copperTree = new CollectibleTree(Collectibles::copper, nullptr, 5, 50, 25, 128, vZero, copperTreeColor, deadCopperTreeColor);
PlacedOnLanding* copperTreeSeed = new PlacedOnLanding(copperTree, "Copper seed", copperTreeColor, 0);
Collectible* cCopperTreeSeed = new Collectible(*copperTreeSeed, vZero);

Color ironTreeColor = Color(67, 90, 99), deadIronTreeColor = Color(45, 47, 48);
CollectibleTree* ironTree = new CollectibleTree(Collectibles::iron, nullptr, 10, 500, 10, 256, vZero, ironTreeColor, deadIronTreeColor);
PlacedOnLanding* ironTreeSeed = new PlacedOnLanding(ironTree, "Iron tree seed", ironTreeColor, 0);
Collectible* cIronTreeSeed = new Collectible(*ironTreeSeed, vZero);

#pragma endregion