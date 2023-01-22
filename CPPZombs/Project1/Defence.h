#include "BuildingBlocks.h"

class CollectibleTree : public FunctionalBlock2
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
		FunctionalBlock2(timePer, pos, dimensions, color, mass, maxHealth, health, name)
	{ }

	void Start() override
	{
		FunctionalBlock2::Start();
		timeSince = timePer * RandFloat();
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

	float TimeIncrease() override
	{
		return game->dTime * game->brightness;
	}

	void DUpdate() override
	{
		if (currentLifespan >= deadStage)
			color = deadColor;
		else if (currentLifespan >= cyclesToGrow)
			color = adultColor;
		FunctionalBlock2::DUpdate();
	}

	bool TUpdate() override
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

	void UIUpdate() override
	{
		Vec2 topLeft = TopLeft();
		if (currentLifespan < cyclesToGrow)
		{
			DrawUIBox(topLeft, topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 15), "Baby " + name, color, deadColor, collectible->color);
			game->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 7), ToStringWithPrecision(
				timePer * (cyclesToGrow - currentLifespan) - timeSince, 1), color);
		}
		else if (currentLifespan < deadStage)
		{
			DrawUIBox(topLeft, topLeft + Vec2(48 + static_cast<int>(name.length()) * 8, 22), "Adult " + name, color, deadColor, collectible->color);
			game->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 7), ToStringWithPrecision(timePer - timeSince, 1), color);
			game->DrawString(topLeft + Vec2(1, 1) + Vec2(0, 14), ToStringWithPrecision(
				timePer * (deadStage - currentLifespan) - timeSince, 1), color);
		}
		else
			DrawUIBox(topLeft, topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 8), "Dead " + name, deadColor, color, collectible->color);
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
	int maxGenerations, generation;

	Vine(Collectible* collectible, Collectible* seed, int cyclesToGrow, int deadStage, int maxGenerations, int chanceForSeed,
		float timePer, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = olc::WHITE, Color adultColor = olc::MAGENTA, Color deadColor = olc::BLACK,
		int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") : 
		CollectibleTree(collectible, seed, cyclesToGrow, deadStage, chanceForSeed, timePer, pos, dimensions, color, adultColor, deadColor, mass, maxHealth, health, name),
		maxGenerations(maxGenerations), generation(0)
	{ }

	Vine(Vine* baseClass, Vec2 dir, Vec2 pos) :
		Vine(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		Vine* newVine = new Vine(this, dir, pos);
		newVine->Start();
		return newVine;
	}

	float TimeIncrease() override
	{
		return game->dTime * (1.0f - game->brightness);
	}

	bool TUpdate() override
	{
		if (generation < maxGenerations && currentLifespan >= cyclesToGrow && currentLifespan < deadStage)
		{
			Vec2 placementPos = pos;
			while (placementPos == pos)
				placementPos = pos + (dimensions * 2 - vOne) * Vec2((rand() % 3) - 1, (rand() % 3) - 1);
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(placementPos, dimensions);
			if (!hitEntities.size())
			{
				Vine* newVine = (Vine*)baseClass->Clone(placementPos, up, this);
				newVine->generation = generation + 1;
				game->entities->push_back(newVine);
			}
		}
		currentLifespan++;
		return true;
	}

	void OnDeath(Entity* damageDealer) override
	{
		CollectibleTree::OnDeath(damageDealer);
		if (rand() % 100 < chanceForSeed)
			game->entities->push_back(seed->Clone(pos));
		else
			game->entities->push_back(collectible->Clone(pos));
	}

	void UIUpdate() override
	{
		Vec2 topLeft = TopLeft();
		if (currentLifespan < cyclesToGrow)
		{
			DrawUIBox(topLeft, topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 22), "Baby " + name, color, deadColor, collectible->color);
			game->DrawString(topLeft + Vec2(1, 8), ToStringWithPrecision(
				timePer * (cyclesToGrow - currentLifespan) - timeSince, 1), color);
			game->DrawString(topLeft + Vec2(1, 15), "Gen " + std::to_string(generation) + "/" + std::to_string(maxGenerations), color);
		}
		else if (currentLifespan < deadStage)
		{
			DrawUIBox(topLeft, topLeft + Vec2(48 + static_cast<int>(name.length()) * 8, 29), "Adult " + name, color, deadColor, collectible->color);
			game->DrawString(topLeft + Vec2(1, 8), ToStringWithPrecision(timePer - timeSince, 1), color);
			game->DrawString(topLeft + Vec2(1, 15), ToStringWithPrecision(
				timePer * (deadStage - currentLifespan) - timeSince, 1), color);
			game->DrawString(topLeft + Vec2(1, 22), "Gen " + std::to_string(generation) + "/" + std::to_string(maxGenerations), color);
		}
		else
		{
			DrawUIBox(topLeft, topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 15), "Dead " + name, deadColor, color, collectible->color);
			game->DrawString(topLeft + Vec2(1, 8), "Gen " + std::to_string(generation) + "/" + std::to_string(maxGenerations), deadColor);
		}
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 topLeft = TopLeft(), bottomRight;
		if (currentLifespan < cyclesToGrow)
		{
			bottomRight = topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 22);
		}
		else if (currentLifespan < deadStage)
		{
			bottomRight = topLeft + Vec2(48 + static_cast<int>(name.length()) * 8, 29);
		}
		else
		{
			bottomRight = topLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 15);
		}
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}
};





#pragma region Plants

namespace Plants
{
	namespace Trees
	{
		Color babyCopperTreeColor = Color(207, 137, 81), copperTreeColor = Color(163, 78, 8), deadCopperTreeColor = Color(94, 52, 17);
		CollectibleTree* copperTree = new CollectibleTree(Collectibles::copper, nullptr, 5, 50, 25, 4.0f, vZero, vOne, babyCopperTreeColor, copperTreeColor, deadCopperTreeColor, 1, 1, 1, "Copper tree");

		Color babyIronTreeColor = Color(96, 192, 225), ironTreeColor = Color(67, 90, 99), deadIronTreeColor = Color(45, 47, 48);
		CollectibleTree* ironTree = new CollectibleTree(Collectibles::iron, nullptr, 10, 100, 10, 8.0f, vZero, vOne, babyIronTreeColor, ironTreeColor, deadIronTreeColor, 1, 1, 1, "Iron tree");

		Color babyRubyTreeColor = Color(207, 120, 156), rubyTreeColor = Color(135, 16, 66), deadRubyTreeColor = Color(120, 65, 88);
		CollectibleTree* rubyTree = new CollectibleTree(Collectibles::ruby, nullptr, 5, 15, 50, 4.0f, vZero, vOne, babyRubyTreeColor, rubyTreeColor, deadRubyTreeColor, 1, 1, 1, "Ruby tree");

		Color babyEmeraldTreeColor = Color(145, 255, 204), emeraldTreeColor = Color(65, 166, 119), deadEmeraldTreeColor = Color(61, 97, 80);
		CollectibleTree* emeraldTree = new CollectibleTree(Collectibles::emerald, nullptr, 5, 15, 50, 4.0f, vZero, vOne, babyEmeraldTreeColor, emeraldTreeColor, deadEmeraldTreeColor, 1, 1, 1, "Emerald tree");

		Color babyRockTreeColor = Color(212, 212, 212), rockTreeColor = Color(201, 196, 165), deadRockTreeColor = Color(150, 140, 78);
		CollectibleTree* rockTree = new CollectibleTree(Collectibles::rock, nullptr, 5, 8, 75, 4.0f, vZero, vOne, babyRockTreeColor, rockTreeColor, deadRockTreeColor, 1, 1, 1, "Rock tree");
	}


	namespace Vines
	{
		Color babyCheeseVineColor = Color(255, 210, 112), cheeseVineColor = Color(200, 160, 75), deadCheeseVineColor = Color(140, 110, 50);
		Vine* cheeseVine = new Vine(Collectibles::cheese, nullptr, 5, 10, 5, 10, 2.0f, vZero, vOne, babyCheeseVineColor, cheeseVineColor, deadCheeseVineColor, 1, 1, 1, "Cheese vine");

		Color babyLeadVineColor = Color(198, 111, 227), leadVineColor = Color(153, 29, 194), deadLeadVineColor = Color(15, 50, 61);
		Vine* leadVine = new Vine(Collectibles::lead, nullptr, 2, 6, 15, 5, 3.0f, vZero, vOne, babyLeadVineColor, leadVineColor, deadLeadVineColor, 2, 1, 1, "Lead vine");
		
		Color babyTopazVineColor = Color(255, 218, 84), topazVineColor = Color(181, 142, 0), deadTopazVineColor = Color(107, 84, 0);
		Vine* topazVine = new Vine(Collectibles::topaz, nullptr, 2, 5, 30, 5, 4.0f, vZero, vOne * 2, babyTopazVineColor, topazVineColor, deadTopazVineColor, 5, 6, 6, "Topaz vine");
	}

	// Keep a list of all of the plants. CollectibleTree is the base of all plants so it's what we'll use for the pointer.
	vector<CollectibleTree*> plants{ Trees::copperTree, Trees::ironTree,
		Trees::rubyTree, Trees::emeraldTree, Trees::rockTree,
		Vines::cheeseVine, Vines::topazVine, Vines::leadVine };
}

namespace Resources::Seeds
{
	// Trees
	PlacedOnLanding* copperTreeSeed = new PlacedOnLanding(Plants::Trees::copperTree, "Copper tree seed", "Seed", Plants::Trees::copperTreeColor, 0);
	PlacedOnLanding* ironTreeSeed = new PlacedOnLanding(Plants::Trees::ironTree, "Iron tree seed", "Seed", Plants::Trees::ironTreeColor, 0);
	PlacedOnLanding* rockTreeSeed = new PlacedOnLanding(Plants::Trees::rockTree, "Rock tree seed", "Seed", Plants::Trees::rockTreeColor, 0);
	CorruptOnKill* rubyTreeSeed = new CorruptOnKill(Plants::Trees::rubyTree, "Ruby tree seed", "Corruption Seed", Plants::Trees::rubyTreeColor, 1);
	CorruptOnKill* emeraldTreeSeed = new CorruptOnKill(Plants::Trees::emeraldTree, "Emerald tree seed", "Corruption Seed", Plants::Trees::emeraldTreeColor, 1);
	// Vines
	PlacedOnLanding* cheeseVineSeed = new PlacedOnLanding(Plants::Vines::cheeseVine, "Cheese vine seed", "Seed", Plants::Vines::cheeseVineColor, 0);
	PlacedOnLanding* topazTreeSeed = new PlacedOnLanding(Plants::Vines::topazVine, "Topaz vine seed", "Seed", Plants::Vines::topazVineColor, 0, 1, 15.0f, false, vOne * 2);
	PlacedOnLanding* leadVineSeed = new PlacedOnLanding(Plants::Vines::leadVine, "Lead vine seed", "Seed", Plants::Vines::leadVineColor, 0);

	// Keep a list of all of the seeds.
	vector<Item*> plantSeeds{ copperTreeSeed, ironTreeSeed, rockTreeSeed, rubyTreeSeed, emeraldTreeSeed, cheeseVineSeed, topazTreeSeed, leadVineSeed };
}

namespace Collectibles::Seeds
{
	// Vines
	Collectible* copperTreeSeed = new Collectible(*Resources::Seeds::copperTreeSeed);
	Collectible* ironTreeSeed = new Collectible(*Resources::Seeds::ironTreeSeed);
	Collectible* rubyTreeSeed = new Collectible(*Resources::Seeds::rubyTreeSeed);
	Collectible* emeraldTreeSeed = new Collectible(*Resources::Seeds::emeraldTreeSeed);
	Collectible* rockTreeSeed = new Collectible(*Resources::Seeds::rockTreeSeed);
	// Vines
	Collectible* cheeseVineSeed = new Collectible(*Resources::Seeds::cheeseVineSeed);
	Collectible* topazTreeSeed = new Collectible(*Resources::Seeds::topazTreeSeed);
	Collectible* leadVindSeed = new Collectible(*Resources::Seeds::leadVineSeed);

	// Keep a list of all of the seeds.
	vector<Collectible*> plantSeeds{ copperTreeSeed, ironTreeSeed, rubyTreeSeed, emeraldTreeSeed, rockTreeSeed, cheeseVineSeed, topazTreeSeed, leadVindSeed };
}

#pragma endregion