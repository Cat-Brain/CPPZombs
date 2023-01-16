#include "BuildingBlocks.h"

class CollectibleTree : public FunctionalBlock
{
public:
	Collectible* collectible, *seed;
	Color adultColor, deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;

	CollectibleTree(Collectible* collectible, Collectible* seed, int cyclesToGrow, int deadStage, int chanceForSeed,
		float timePer, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = olc::WHITE, Color adultColor = olc::MAGENTA, Color deadColor = olc::BLACK,
		int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(seed), cyclesToGrow(cyclesToGrow), deadStage(deadStage),
		currentLifespan(0), chanceForSeed(chanceForSeed), adultColor(adultColor), deadColor(deadColor),
		FunctionalBlock(timePer, (float)rand() / RAND_MAX * timePer,
			pos, dimensions, color, mass, maxHealth, health, name)
	{ }

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

	void DUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime)
	{
		Color oldColor = color;
		if (currentLifespan >= deadStage)
			color = deadColor;
		else if (currentLifespan >= cyclesToGrow)
			color = adultColor;
		FunctionalBlock::DUpdate(game, entities, frameCount, inputs, dTime);
	}

	bool TUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime)
	{
		if (currentLifespan >= cyclesToGrow && currentLifespan < deadStage)
		{
			if (rand() % 100 < chanceForSeed)
				entities->push_back(seed->Clone(pos + Vec2((rand() % 2) * 2 - 1, (rand() % 2) * 2 - 1)));
			else
				entities->push_back(collectible->Clone(pos + Vec2((rand() % 2) * 2 - 1, (rand() % 2) * 2 - 1)));
		}
		currentLifespan++;
		return true;
	}

	void UIUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		Vec2 topLeft = TopLeft();
		if (currentLifespan < cyclesToGrow)
		{
			DrawUIBox(game, topLeft, topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 15), "Baby " + name, color, deadColor, collectible->color);
			game->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 7), ToStringWithPrecision(
				timePer * cyclesToGrow - (tTime - lastTime + timePer * currentLifespan), 1), color);
		}
		else if (currentLifespan < deadStage)
		{
			DrawUIBox(game, topLeft, topLeft + Vec2(48 + static_cast<int>(name.length()) * 8, 22), "Adult " + name, color, deadColor, collectible->color);
			game->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 7), ToStringWithPrecision(timePer - tTime + lastTime, 1), color);
			game->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 14), ToStringWithPrecision(
				timePer * deadStage - (tTime - lastTime + timePer * currentLifespan), 1), color);
		}
		else
			DrawUIBox(game, topLeft, topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 8), "Dead " + name, deadColor, color, collectible->color);
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 topLeft = TopLeft(), bottomRight;
		if (currentLifespan < cyclesToGrow)
		{
			bottomRight = topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 15);
		}
		else if (currentLifespan < deadStage)
		{
			bottomRight = topLeft + Vec2(48 + static_cast<int>(name.length()) * 8, 22);
		}
		else
		{
			bottomRight = topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 8);
		}
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}
};




#pragma region Trees

namespace Trees
{
	Color babyCopperTreeColor = Color(207, 137, 81), copperTreeColor = Color(163, 78, 8), deadCopperTreeColor = Color(94, 52, 17);
	CollectibleTree* copperTree = new CollectibleTree(Collectibles::copper, nullptr, 5, 50, 25, 4.0f, vZero, vOne, babyCopperTreeColor, copperTreeColor, deadCopperTreeColor, 1, 1, 1, "Copper tree");

	Color babyIronTreeColor = Color(96, 192, 225), ironTreeColor = Color(67, 90, 99), deadIronTreeColor = Color(45, 47, 48);
	CollectibleTree* ironTree = new CollectibleTree(Collectibles::iron, nullptr, 10, 500, 10, 8.0f, vZero, vOne, babyIronTreeColor, ironTreeColor, deadIronTreeColor, 1, 1, 1, "Iron tree");

	Color babyRubyTreeColor = Color(207, 120, 156), rubyTreeColor = Color(135, 16, 66), deadRubyTreeColor = Color(120, 65, 88);
	CollectibleTree* rubyTree = new CollectibleTree(Collectibles::ruby, nullptr, 5, 15, 50, 4.0f, vZero, vOne, babyRubyTreeColor, rubyTreeColor, deadRubyTreeColor, 1, 1, 1, "Ruby tree");

	Color babyEmeraldTreeColor = Color(145, 255, 204), emeraldTreeColor = Color(65, 166, 119), deadEmeraldTreeColor = Color(61, 97, 80);
	CollectibleTree* emeraldTree = new CollectibleTree(Collectibles::emerald, nullptr, 5, 15, 50, 4.0f, vZero, vOne, babyEmeraldTreeColor, emeraldTreeColor, deadEmeraldTreeColor, 1, 1, 1, "Emerald tree");

	Color babyCheeseTreeColor = Color(255, 210, 112), cheeseTreeColor = Color(200, 160, 75), deadCheeseTreeColor = Color(140, 110, 50);
	CollectibleTree* cheeseTree = new CollectibleTree(Collectibles::cheese, nullptr, 5, 25, 10, 2.0f, vZero, vOne, babyCheeseTreeColor, cheeseTreeColor, deadCheeseTreeColor, 1, 1, 1, "Cheese tree");
}

namespace Resources::Seeds
{
	PlacedOnLanding* copperTreeSeed = new PlacedOnLanding(Trees::copperTree, "Copper tree seed", "Seed", Trees::copperTreeColor, 0);
	PlacedOnLanding* ironTreeSeed = new PlacedOnLanding(Trees::ironTree, "Iron tree seed", "Seed", Trees::ironTreeColor, 0);
	CorruptOnKill* rubyTreeSeed = new CorruptOnKill(Trees::rubyTree, "Ruby tree seed", "Seed", Trees::rubyTreeColor, 1);
	CorruptOnKill* emeraldTreeSeed = new CorruptOnKill(Trees::emeraldTree, "Emerald tree seed", "Seed", Trees::emeraldTreeColor, 1);
	PlacedOnLanding* cheeseTreeSeed = new PlacedOnLanding(Trees::cheeseTree, "Cheese tree seed", "Seed", Trees::cheeseTreeColor, 0);
}

namespace Collectibles::Seeds
{
	Collectible* copperTreeSeed = new Collectible(*Resources::Seeds::copperTreeSeed);
	Collectible* ironTreeSeed = new Collectible(*Resources::Seeds::ironTreeSeed);
	Collectible* rubyTreeSeed = new Collectible(*Resources::Seeds::rubyTreeSeed);
	Collectible* emeraldTreeSeed = new Collectible(*Resources::Seeds::emeraldTreeSeed);
	Collectible* cheeseTreeSeed = new Collectible(*Resources::Seeds::cheeseTreeSeed);
}
#pragma endregion

class Turret : public Duct
{
public:
	Items items;
	float range;
	bool overShoot;

	Turret(float range, bool overShoot, float timePer, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Duct(timePer, pos, dimensions, color, mass, maxHealth, health, name), range(range), overShoot(overShoot)
	{ }

	void Update(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		printf("?");
		vector<Entity*> newCollectibles = EntitiesAtPos(pos, entities->collectibles);
		for (Entity* collectible : newCollectibles)
		{
			items.push_back(((Collectible*)collectible)->baseItem);
			((Entities*)entities)->Remove(collectible);
		}
		for (Entity* collectible : containedEntities)
		{
			items.push_back(((Collectible*)collectible)->baseItem);
			((Entities*)entities)->Remove(collectible);
		}
		containedEntities.clear();

		FunctionalBlock::Update(game, entities, frameCount, inputs, dTime);
	}

	bool TUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		Entity* entity = entities->FindNearestEnemy(pos);

		Item shotItem;
		if (Distance(pos, entity->pos) > range || !items.TryTakeIndex(0, shotItem))
			return false;
		
		entities->push_back(new ShotItem(basicShotItem, shotItem, pos, overShoot ? (Vec2)(Normalized(entity->pos - pos) * range) : entity->pos - pos, this));

		return true;
	}
};

namespace Structures
{
	namespace Defence
	{
		Turret* basicTurret = new Turret(30.0f, false, 0.5f, vZero, vOne, Color(194, 107, 54), 1, 3, 3, "Basic turret");
	}
}