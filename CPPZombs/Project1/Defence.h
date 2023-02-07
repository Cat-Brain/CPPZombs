#include "BuildingBlocks.h"

class CollectibleTree : public FunctionalBlock2
{
public:
	Collectible* collectible, *seed;
	RGBA adultColor, deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;

	CollectibleTree(Collectible* collectible, Collectible* seed, int cyclesToGrow, int deadStage, int chanceForSeed,
		float timePer, Vec2 pos = vZero, Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA adultColor = RGBA(),
		RGBA deadColor = RGBA(), RGBA subsurfaceResistance = RGBA(),
		int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(seed), cyclesToGrow(cyclesToGrow), deadStage(deadStage),
		currentLifespan(0), chanceForSeed(chanceForSeed), adultColor(adultColor), deadColor(deadColor),
		FunctionalBlock2(timePer, pos, dimensions, color, subsurfaceResistance, mass, maxHealth, health, name)
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

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<CollectibleTree>(this, dir, pos);
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
				game->entities->push_back(seed->Clone(pos + Vec2f(dimensions + seed->dimensions) * 0.5f * Vec2f((rand() % 2) * 2 - 1, (rand() % 2) * 2 - 1)));
			else
				game->entities->push_back(collectible->Clone(pos + (dimensions + collectible->dimensions) * 0.5f * Vec2f((rand() % 2) * 2 - 1, (rand() % 2) * 2 - 1)));
		}
		currentLifespan++;
		return true;
	}

	void UIUpdate() override
	{
		Vec2f bottomLeft = BottomLeft();
		if (currentLifespan < cyclesToGrow)
		{
			DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Baby " + name) * COMMON_TEXT_SCALE / font.minimumSize / 2, COMMON_TEXT_SCALE),
				"Baby " + name, color, deadColor, collectible->color);
			game->DrawString(ToStringWithPrecision(timePer * (cyclesToGrow - currentLifespan) - timeSince, 1),
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE));
		}
		else if (currentLifespan < deadStage)
		{
			DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Adult " + name) * COMMON_TEXT_SCALE / font.minimumSize / 2, COMMON_TEXT_SCALE * 3 / 2),
				"Adult " + name, color, deadColor, collectible->color);
			game->DrawString(ToStringWithPrecision(timePer - timeSince, 1),
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE));
			game->DrawString(ToStringWithPrecision(timePer * (deadStage - currentLifespan) - timeSince, 1),
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE * 2));
		}
		else
			DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Dead " + name) * COMMON_TEXT_SCALE / font.minimumSize / 2, COMMON_TEXT_SCALE / 2),
				"Dead " + name, deadColor, color, collectible->color);
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 bottomLeft = BottomLeft(), topRight;
		if (currentLifespan < cyclesToGrow)
		{
			topRight = bottomLeft + Vec2f(40 + static_cast<int>(name.length()) * 8, 15);
		}
		else if (currentLifespan < deadStage)
		{
			topRight = bottomLeft + Vec2f(48 + static_cast<int>(name.length()) * 8, 22);
		}
		else
		{
			topRight = bottomLeft + Vec2f(40 + static_cast<int>(name.length()) * 8, 8);
		}
		return screenSpacePos.x >= bottomLeft.x && screenSpacePos.x <= topRight.x &&
			screenSpacePos.y >= bottomLeft.y && screenSpacePos.y <= topRight.y;
	}
};




class Vine : public CollectibleTree
{
public:
	int maxGenerations, generation;

	Vine(Collectible* collectible, Collectible* seed, int cyclesToGrow, int deadStage, int maxGenerations, int chanceForSeed,
		float timePer, Vec2 pos = vZero, Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA adultColor = RGBA(),
		RGBA deadColor = RGBA(), RGBA subsurfaceResistance = RGBA(),
		int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") : 
		CollectibleTree(collectible, seed, cyclesToGrow, deadStage, chanceForSeed, timePer, pos, dimensions, color, adultColor, deadColor, subsurfaceResistance, mass, maxHealth, health, name),
		maxGenerations(maxGenerations), generation(0)
	{ }

	Vine(Vine* baseClass, Vec2 dir, Vec2 pos) :
		Vine(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Vine>(this, dir, pos);
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
				placementPos = pos + dimensions * Vec2((rand() % 3) - 1, (rand() % 3) - 1);
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(placementPos, dimensions);
			if (!hitEntities.size())
			{
				unique_ptr<Entity> newVine = baseClass->Clone(placementPos, up, this);
				((Vine*)newVine.get())->generation = generation + 1;
				game->entities->push_back(std::move(newVine));
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
		Vec2f bottomLeft = BottomLeft();
		if (currentLifespan < cyclesToGrow)
		{
			DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Baby " + name) * COMMON_TEXT_SCALE / font.minimumSize / 2, COMMON_TEXT_SCALE * 3 / 2),
				"Baby " + name, color, deadColor, collectible->color);
			game->DrawString(ToStringWithPrecision(timePer * (cyclesToGrow - currentLifespan) - timeSince, 1),
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE));
			game->DrawString("Gen " + to_string(generation) ,
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE * 2));
		}
		else if (currentLifespan < deadStage)
		{
			DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Adult " + name) * COMMON_TEXT_SCALE / font.minimumSize / 2, COMMON_TEXT_SCALE * 2),
				"Adult " + name, color, deadColor, collectible->color);
			game->DrawString(ToStringWithPrecision(timePer - timeSince, 1),
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE));
			game->DrawString(ToStringWithPrecision(timePer * (deadStage - currentLifespan) - timeSince, 1),
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE * 2));
			game->DrawString("Gen " + to_string(generation),
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE * 3));
		}
		else
		{
			DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Dead " + name) * COMMON_TEXT_SCALE / font.minimumSize / 2, COMMON_TEXT_SCALE),
				"Dead " + name, deadColor, color, collectible->color);
			game->DrawString("Gen " + to_string(generation),
				pos + right + dimensions / 2, COMMON_TEXT_SCALE, color, Vec2(0, COMMON_TEXT_SCALE));
		}
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 bottomLeft = BottomLeft(), topRight;
		if (currentLifespan < cyclesToGrow)
		{
			topRight = bottomLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 22);
		}
		else if (currentLifespan < deadStage)
		{
			topRight = bottomLeft + Vec2(48 + static_cast<int>(name.length()) * 8, 29);
		}
		else
		{
			topRight = bottomLeft + Vec2(40 + static_cast<int>(name.length()) * 8, 15);
		}
		return screenSpacePos.x >= bottomLeft.x && screenSpacePos.x <= topRight.x &&
			screenSpacePos.y >= bottomLeft.y && screenSpacePos.y <= topRight.y;
	}
};





#pragma region Plants

namespace Plants
{
	namespace Trees
	{
		RGBA babyCopperTreeColor = RGBA(207, 137, 81), copperTreeColor = RGBA(163, 78, 8), deadCopperTreeColor = RGBA(94, 52, 17), copperResistence = RGBA(0, 0, 50);
		CollectibleTree* copperTree = new CollectibleTree(Collectibles::copper, nullptr, 5, 50, 25, 4.0f, vZero, vOne, babyCopperTreeColor, copperTreeColor, deadCopperTreeColor, copperResistence, 1, 1, 1, "Copper tree");

		RGBA babyIronTreeColor = RGBA(96, 192, 225), ironTreeColor = RGBA(67, 90, 99), deadIronTreeColor = RGBA(45, 47, 48), ironResistence = RGBA(50, 50, 0);
		CollectibleTree* ironTree = new CollectibleTree(Collectibles::iron, nullptr, 10, 100, 10, 8.0f, vZero, vOne, babyIronTreeColor, ironTreeColor, deadIronTreeColor, ironResistence, 1, 1, 1, "Iron tree");

		RGBA babyRubyTreeColor = RGBA(207, 120, 156), rubyTreeColor = RGBA(135, 16, 66), deadRubyTreeColor = RGBA(120, 65, 88), rubyResistence = RGBA(0, 50, 50);
		CollectibleTree* rubyTree = new CollectibleTree(Collectibles::ruby, nullptr, 5, 15, 50, 4.0f, vZero, vOne, babyRubyTreeColor, rubyTreeColor, deadRubyTreeColor, rubyResistence, 1, 1, 1, "Ruby tree");

		RGBA babyEmeraldTreeColor = RGBA(145, 255, 204), emeraldTreeColor = RGBA(65, 166, 119), deadEmeraldTreeColor = RGBA(61, 97, 80), emeraldResistence = RGBA(50, 0, 50);
		CollectibleTree* emeraldTree = new CollectibleTree(Collectibles::emerald, nullptr, 5, 15, 50, 4.0f, vZero, vOne, babyEmeraldTreeColor, emeraldTreeColor, deadEmeraldTreeColor, emeraldResistence, 1, 1, 1, "Emerald tree");

		RGBA babyRockTreeColor = RGBA(212, 212, 212), rockTreeColor = RGBA(201, 196, 165), deadRockTreeColor = RGBA(150, 140, 78), rockResistence = RGBA(50, 0, 50);
		CollectibleTree* rockTree = new CollectibleTree(Collectibles::rock, nullptr, 5, 8, 75, 4.0f, vZero, vOne, babyRockTreeColor, rockTreeColor, deadRockTreeColor, rockResistence, 1, 1, 1, "Rock tree");
	}


	namespace Vines
	{
		RGBA babyCheeseVineColor = RGBA(255, 210, 112), cheeseVineColor = RGBA(200, 160, 75), deadCheeseVineColor = RGBA(140, 110, 50), cheeseResistence = RGBA(0, 0, 50);
		Vine* cheeseVine = new Vine(Collectibles::cheese, nullptr, 5, 10, 5, 10, 2.0f, vZero, vOne, babyCheeseVineColor, cheeseVineColor, deadCheeseVineColor, cheeseResistence, 1, 1, 1, "Cheese vine");

		RGBA babyLeadVineColor = RGBA(198, 111, 227), leadVineColor = RGBA(153, 29, 194), deadLeadVineColor = RGBA(15, 50, 61), leadResistence = RGBA(0, 50, 0);
		Vine* leadVine = new Vine(Collectibles::lead, nullptr, 2, 6, 15, 5, 3.0f, vZero, vOne, babyLeadVineColor, leadVineColor, deadLeadVineColor, leadResistence, 2, 1, 1, "Lead vine");
		
		RGBA babyTopazVineColor = RGBA(255, 218, 84), topazVineColor = RGBA(181, 142, 0), deadTopazVineColor = RGBA(107, 84, 0), topazResistence = RGBA(25, 0, 50);
		Vine* topazVine = new Vine(Collectibles::topaz, nullptr, 2, 5, 30, 5, 4.0f, vZero, vOne * 3, babyTopazVineColor, topazVineColor, deadTopazVineColor, topazResistence, 5, 6, 6, "Topaz vine");
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
	PlacedOnLanding* topazTreeSeed = new PlacedOnLanding(Plants::Vines::topazVine, "Topaz vine seed", "Seed", Plants::Vines::topazVineColor, 0, 1, 15.0f, false, vOne * 3);
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