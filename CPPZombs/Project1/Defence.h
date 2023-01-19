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

	void DUpdate(Game* game, float dTime) override
	{
		Color oldColor = color;
		if (currentLifespan >= deadStage)
			color = deadColor;
		else if (currentLifespan >= cyclesToGrow)
			color = adultColor;
		FunctionalBlock::DUpdate(game, dTime);
	}

	bool TUpdate(Game* game, float dTime) override
	{
		if (currentLifespan >= cyclesToGrow && currentLifespan < deadStage)
		{
			if (rand() % 100 < chanceForSeed)
				game->entities->push_back(seed->Clone(pos + (dimensions + collectible->dimensions - vOne) * Vec2((rand() % 2) * 2 - 1, (rand() % 2) * 2 - 1)));
			else
				game->entities->push_back(collectible->Clone(pos + (dimensions + seed->dimensions - vOne) * Vec2((rand() % 2) * 2 - 1, (rand() % 2) * 2 - 1)));
		}
		currentLifespan++;
		return true;
	}

	void UIUpdate(Game* game, float dTime) override
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




class Vine : public CollectibleTree
{
public:
	using CollectibleTree::CollectibleTree;

	void Start() override
	{
		lastTime = tTime + RandFloat() * timePer;
	}


	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		Vine* newVine = new Vine(this, dir, pos);
		newVine->Start();
		return newVine;
	}

	bool TUpdate(Game* game, float dTime) override
	{
		if (currentLifespan >= cyclesToGrow && currentLifespan < deadStage)
		{
			Vec2 placementPos = pos;
			while (placementPos == pos)
				placementPos = pos + (dimensions * 2 - vOne) * Vec2((rand() % 3) - 1, (rand() % 3) - 1);
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(placementPos, dimensions);
			if (!hitEntities.size() && game->entities->size() < 3600)
				game->entities->push_back(baseClass->Clone(placementPos));
		}
		if (currentLifespan > 50)
			DestroySelf(game, this);
		currentLifespan++;
		return true;
	}

	void OnDeath(Entities* entities, Entity* damageDealer) override
	{
		CollectibleTree::OnDeath(entities, damageDealer);
		if (rand() % 100 < chanceForSeed)
			entities->push_back(seed->Clone(pos));
		else
			entities->push_back(collectible->Clone(pos));
	}
};





#pragma region Trees

namespace Plants
{
	namespace Trees
	{
		Color babyCopperTreeColor = Color(207, 137, 81), copperTreeColor = Color(163, 78, 8), deadCopperTreeColor = Color(94, 52, 17);
		CollectibleTree* copperTree = new CollectibleTree(Collectibles::copper, nullptr, 5, 50, 25, 4.0f, vZero, vOne, babyCopperTreeColor, copperTreeColor, deadCopperTreeColor, 1, 1, 1, "Copper tree");

		Color babyIronTreeColor = Color(96, 192, 225), ironTreeColor = Color(67, 90, 99), deadIronTreeColor = Color(45, 47, 48);
		CollectibleTree* ironTree = new CollectibleTree(Collectibles::iron, nullptr, 10, 100, 10, 8.0f, vZero, vOne, babyIronTreeColor, ironTreeColor, deadIronTreeColor, 1, 1, 1, "Iron tree");

		Color babyCheeseTreeColor = Color(255, 210, 112), cheeseTreeColor = Color(200, 160, 75), deadCheeseTreeColor = Color(140, 110, 50);
		CollectibleTree* cheeseTree = new CollectibleTree(Collectibles::cheese, nullptr, 5, 25, 10, 2.0f, vZero, vOne, babyCheeseTreeColor, cheeseTreeColor, deadCheeseTreeColor, 1, 1, 1, "Cheese tree");

		Color babyRubyTreeColor = Color(207, 120, 156), rubyTreeColor = Color(135, 16, 66), deadRubyTreeColor = Color(120, 65, 88);
		CollectibleTree* rubyTree = new CollectibleTree(Collectibles::ruby, nullptr, 5, 15, 50, 4.0f, vZero, vOne, babyRubyTreeColor, rubyTreeColor, deadRubyTreeColor, 1, 1, 1, "Ruby tree");

		Color babyEmeraldTreeColor = Color(145, 255, 204), emeraldTreeColor = Color(65, 166, 119), deadEmeraldTreeColor = Color(61, 97, 80);
		CollectibleTree* emeraldTree = new CollectibleTree(Collectibles::emerald, nullptr, 5, 15, 50, 4.0f, vZero, vOne, babyEmeraldTreeColor, emeraldTreeColor, deadEmeraldTreeColor, 1, 1, 1, "Emerald tree");
		// 3x3 but not a corruption seed.
		Color babyTopazTreeColor = Color(255, 218, 84), topazTreeColor = Color(181, 142, 0), deadTopazTreeColor = Color(107, 84, 0);
		CollectibleTree* topazTree = new CollectibleTree(Collectibles::topaz, nullptr, 5, 50, 10, 4.0f, vZero, vOne * 2, babyTopazTreeColor, topazTreeColor, deadTopazTreeColor, 5, 6, 6, "Topaz tree");
	}


	namespace Vines
	{
		Color babyLeadVineColor = Color(198, 111, 227), leadVineColor = Color(153, 29, 194), deadLeadVineColor = Color(15, 50, 61);
		Vine* leadVine = new Vine(Collectibles::lead, nullptr, 2, 6, 5, 3.0f, vZero, vOne, babyLeadVineColor, leadVineColor, deadLeadVineColor, 2, 1, 1, "Lead vine");
	}

	// Keep a list of all of the plants.
	vector<CollectibleTree*> plants{ Trees::copperTree, Trees::ironTree, Trees::cheeseTree,
		Trees::rubyTree, Trees::emeraldTree, Trees::topazTree,
	Vines::leadVine };
}

namespace Resources::Seeds
{
	PlacedOnLanding* copperTreeSeed = new PlacedOnLanding(Plants::Trees::copperTree, "Copper tree seed", "Seed", Plants::Trees::copperTreeColor, 0);
	PlacedOnLanding* ironTreeSeed = new PlacedOnLanding(Plants::Trees::ironTree, "Iron tree seed", "Seed", Plants::Trees::ironTreeColor, 0);
	PlacedOnLanding* cheeseTreeSeed = new PlacedOnLanding(Plants::Trees::cheeseTree, "Cheese tree seed", "Seed", Plants::Trees::cheeseTreeColor, 0);
	CorruptOnKill* rubyTreeSeed = new CorruptOnKill(Plants::Trees::rubyTree, "Ruby tree seed", "Corruption Seed", Plants::Trees::rubyTreeColor, 1);
	CorruptOnKill* emeraldTreeSeed = new CorruptOnKill(Plants::Trees::emeraldTree, "Emerald tree seed", "Corruption Seed", Plants::Trees::emeraldTreeColor, 1);
	PlacedOnLanding* topazTreeSeed = new PlacedOnLanding(Plants::Trees::topazTree, "Topaz tree seed", "Seed", Plants::Trees::topazTreeColor, 0, 1, 15.0f, vOne * 2);
	PlacedOnLanding* leadVineSeed = new PlacedOnLanding(Plants::Vines::leadVine, "Lead vine seed", "Seed", Plants::Vines::leadVineColor, 0);

	// Keep a list of all of the seeds.
	vector<Item*> plantSeeds{ copperTreeSeed, ironTreeSeed, cheeseTreeSeed, rubyTreeSeed, emeraldTreeSeed, topazTreeSeed, leadVineSeed };
}

namespace Collectibles::Seeds
{
	Collectible* copperTreeSeed = new Collectible(*Resources::Seeds::copperTreeSeed);
	Collectible* ironTreeSeed = new Collectible(*Resources::Seeds::ironTreeSeed);
	Collectible* cheeseTreeSeed = new Collectible(*Resources::Seeds::cheeseTreeSeed);
	Collectible* rubyTreeSeed = new Collectible(*Resources::Seeds::rubyTreeSeed);
	Collectible* emeraldTreeSeed = new Collectible(*Resources::Seeds::emeraldTreeSeed);
	Collectible* topazTreeSeed = new Collectible(*Resources::Seeds::topazTreeSeed);
	Collectible* leadVindSeed = new Collectible(*Resources::Seeds::leadVineSeed);

	// Keep a list of all of the seeds.
	vector<Collectible*> plantSeeds{ copperTreeSeed, ironTreeSeed, cheeseTreeSeed, rubyTreeSeed, emeraldTreeSeed, topazTreeSeed, leadVindSeed };
}
#pragma endregion