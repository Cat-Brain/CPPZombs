#include "BuildingBlocks.h"


float difficultyGrowthModifier[] = { 2.0f, 1.0f, 0.5f };

class Tree : public FunctionalBlock2
{
public:
	Collectible* collectible, *seed;
	RGBA adultColor, deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;
	float babyRadius, maxRadius, babyMass, maxMass;
	float nextPlacementRotation = 0.0f;
	bool nextSpawnSeed = true;

	Tree(Collectible* collectible, int cyclesToGrow, int deadStage, int chanceForSeed,
		float timePer, iVec2 pos = vZero, float radius = 0.5f, float maxRadius = 0.5f, RGBA color = RGBA(), RGBA adultColor = RGBA(), RGBA deadColor = RGBA(),
		float mass = 1, float maxMass = 2, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(nullptr), cyclesToGrow(cyclesToGrow), deadStage(deadStage),
		currentLifespan(0), chanceForSeed(chanceForSeed), adultColor(adultColor), deadColor(deadColor),
		babyRadius(radius), maxRadius(maxRadius), babyMass(mass), maxMass(maxMass),
		FunctionalBlock2(timePer, pos, radius, color, mass, maxHealth, health, name)
	{
		dUpdate = DUPDATE::TREEDU;
		uiUpdate = UIUPDATE::TREEUIU;
		tUpdate = TUPDATE::TREETU;
	}

	Tree(Tree* baseClass, Vec2 dir, Vec2 pos) :
		Tree(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Tree>(this, dir, pos);
	}

	float TimeIncrease() override
	{
		return game->dTime * game->BrightnessAtPos(pos) * difficultyGrowthModifier[game->difficulty];
	}
};

namespace DUpdates
{
	void TreeDU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);
		float lerpValue = min(1.f, (tree->currentLifespan + tree->timeSince / tree->timePer) / tree->cyclesToGrow);
		tree->SetRadius(tree->babyRadius + lerpValue * (tree->maxRadius - tree->babyRadius));
		tree->mass = tree->babyMass + lerpValue * (tree->maxMass - tree->babyMass);

		if (tree->currentLifespan >= tree->deadStage)
			tree->color = tree->deadColor;
		else if (tree->currentLifespan >= tree->cyclesToGrow)
		{
			Collectible* collectible = tree->nextSpawnSeed ? tree->seed : tree->collectible;
			float collectibleRadius = collectible->radius * tree->timeSince / tree->timePer;
			game->DrawCircle(tree->pos + (tree->radius + collectibleRadius) * CircPoint(tree->nextPlacementRotation), collectible->color, collectibleRadius);
			tree->color = tree->adultColor;
		}
		tree->DUpdate(DUPDATE::ENTITYDU);
	}
}


class Vine : public Tree
{
public:
	int maxGenerations, generation;

	Vine(Collectible* collectible, int cyclesToGrow, int deadStage, int maxGenerations, int chanceForSeed,
		float timePer, iVec2 pos = vZero, float radius = 0.5f, RGBA color = RGBA(), RGBA adultColor = RGBA(),
		RGBA deadColor = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Tree(collectible, cyclesToGrow, deadStage, chanceForSeed, timePer, pos, radius, radius, color, adultColor, deadColor,
			mass, mass, maxHealth, health, name),
		maxGenerations(maxGenerations), generation(0)
	{
		uiUpdate = UIUPDATE::VINEUIU;
		onDeath = ONDEATH::VINEOD;
		tUpdate = TUPDATE::VINETU;
	}

	void Start() override
	{
		Tree::Start();
		timeSince = timePer * RandFloat();
	}

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
		return game->dTime * (1.0f - game->BrightnessAtPos(pos)) * difficultyGrowthModifier[game->difficulty];
	}
};

namespace OnDeaths
{
	void VineOD(Entity* entity, Entity* damageDealer)
	{
		Vine* vine = static_cast<Vine*>(entity);
		if (rand() % 100 < vine->chanceForSeed)
			game->entities->push_back(vine->seed->Clone(vine->pos));
		else
			game->entities->push_back(vine->collectible->Clone(vine->pos));
	}
}


namespace TUpdates
{
	bool TreeTU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);
		if (tree->currentLifespan >= tree->cyclesToGrow && tree->currentLifespan < tree->deadStage)
		{
			if (tree->nextSpawnSeed)
				game->entities->push_back(tree->seed->Clone(tree->pos + (tree->radius + tree->seed->radius) * CircPoint(tree->nextPlacementRotation)));
			else
				game->entities->push_back(tree->collectible->Clone(tree->pos + (tree->radius + tree->collectible->radius) * CircPoint(tree->nextPlacementRotation)));
		}
		tree->currentLifespan++;
		tree->nextPlacementRotation = RandFloat() * 2 * PI_F;
		tree->nextSpawnSeed = rand() % 100 < tree->chanceForSeed;
		return true;
	}

	bool VineTU(Entity* entity)
	{
		Vine* vine = static_cast<Vine*>(entity);
		if (vine->generation >= vine->maxGenerations)
			vine->currentLifespan = vine->deadStage;
		if (vine->currentLifespan >= vine->cyclesToGrow && vine->currentLifespan < vine->deadStage)
		{
			Vec2 placementPos = vine->pos + vine->radius * 2 * CircPoint(vine->nextPlacementRotation);
			unique_ptr<Entity> newVine = vine->baseClass->Clone(placementPos, up, vine);
			((Vine*)newVine.get())->generation = vine->generation + 1;
			game->entities->push_back(std::move(newVine));
		}
		vine->currentLifespan++;
		vine->nextPlacementRotation = RandFloat() * 2 * PI_F;
		vine->nextSpawnSeed = true;
		return true;
	}
}


namespace UIUpdates
{
	void TreeUIU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);

		iVec2 bottomLeft = tree->BottomLeft();
		if (tree->currentLifespan < tree->cyclesToGrow)
		{
			tree->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Baby " + tree->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp / 2),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Baby " + tree->name, tree->color, tree->deadColor, tree->collectible->color);
			font.Render(ToStringWithPrecision(tree->timePer * (tree->cyclesToGrow - tree->currentLifespan) - tree->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), tree->color);
		}
		else if (tree->currentLifespan < tree->deadStage)
		{
			tree->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Adult " + tree->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp * 3 / 4),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Adult " + tree->name, tree->color, tree->deadColor, tree->collectible->color);
			font.Render(ToStringWithPrecision(tree->timePer - tree->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), tree->color);
			font.Render(ToStringWithPrecision(tree->timePer * (tree->deadStage - tree->currentLifespan) - tree->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), tree->color);
		}
		else
			tree->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Dead " + tree->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp / 4),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Dead " + tree->name, tree->deadColor, tree->color, tree->collectible->color);
	}

	void VineUIU(Entity* entity)
	{
		Vine* vine = static_cast<Vine*>(entity);
		iVec2 bottomLeft = vine->BottomLeft();
		if (vine->currentLifespan < vine->cyclesToGrow)
		{
			vine->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Baby " + vine->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp * 3 / 4),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Baby " + vine->name, vine->color, vine->deadColor, vine->collectible->color);
			font.Render(ToStringWithPrecision(vine->timePer * (vine->cyclesToGrow - vine->currentLifespan) - vine->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
			font.Render("Gen " + to_string(vine->generation) + " / " + to_string(vine->maxGenerations), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
		}
		else if (vine->currentLifespan < vine->deadStage)
		{
			vine->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Adult " + vine->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Adult " + vine->name, vine->color, vine->deadColor, vine->collectible->color);
			font.Render(ToStringWithPrecision(vine->timePer - vine->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
			font.Render(ToStringWithPrecision(vine->timePer * (vine->deadStage - vine->currentLifespan) - vine->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
			font.Render("Gen " + to_string(vine->generation) + " / " + to_string(vine->maxGenerations), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp * 3 / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
		}
		else
		{
			vine->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Dead " + vine->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp / 2),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Dead " + vine->name, vine->deadColor, vine->color, vine->collectible->color);
			font.Render("Gen " + to_string(vine->generation) + " / " + to_string(vine->maxGenerations), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
		}
	}
}


#pragma region Plants

namespace Plants
{
	namespace Trees
	{
		RGBA babyCopperTreeColor = RGBA(207, 137, 81), copperTreeColor = RGBA(163, 78, 8), deadCopperTreeColor = RGBA(94, 52, 17);
		Tree* copperTree = new Tree(Collectibles::copper, 5, 25, 25, 4.0f, vZero, 0.1f, 1.5f, babyCopperTreeColor, copperTreeColor, deadCopperTreeColor, 0.2f, 3.0f, 1, 1, "Copper tree");

		RGBA babyIronTreeColor = RGBA(96, 192, 225), ironTreeColor = RGBA(67, 90, 99), deadIronTreeColor = RGBA(45, 47, 48);
		Tree* ironTree = new Tree(Collectibles::iron, 120, 180, 10, 0.5f, vZero, 0.1f, 1.5f, babyIronTreeColor, ironTreeColor, deadIronTreeColor, 0.2f, 3.0f, 1, 1, "Iron tree");

		RGBA babyRubyTreeColor = RGBA(207, 120, 156), rubyTreeColor = RGBA(135, 16, 66), deadRubyTreeColor = RGBA(120, 65, 88);
		Tree* rubyTree = new Tree(Collectibles::ruby, 5, 15, 50, 4.0f, vZero, 0.1f, 1.5f, babyRubyTreeColor, rubyTreeColor, deadRubyTreeColor, 0.2f, 3.0f, 1, 1, "Ruby tree");

		RGBA babyEmeraldTreeColor = RGBA(145, 255, 204), emeraldTreeColor = RGBA(65, 166, 119), deadEmeraldTreeColor = RGBA(61, 97, 80);
		Tree* emeraldTree = new Tree(Collectibles::emerald, 5, 15, 50, 4.0f, vZero, 0.1f, 1.5f, babyEmeraldTreeColor, emeraldTreeColor, deadEmeraldTreeColor, 0.2f, 3.0f, 1, 1, "Emerald tree");

		RGBA babyRockTreeColor = RGBA(212, 212, 212), rockTreeColor = RGBA(201, 196, 165), deadRockTreeColor = RGBA(130, 130, 130);
		Tree* rockTree = new Tree(Collectibles::rock, 5, 8, 75, 4.0f, vZero, 0.1f, 1.5f, babyRockTreeColor, rockTreeColor, deadRockTreeColor, 0.2f, 3.0f, 1, 1, "Rock tree");
		
		RGBA babyShadeTreeColor = RGBA(50, 50), shadeTreeColor = RGBA(25, 25), deadShadeTreeColor = RGBA();
		Tree* shadeTree = new Tree(Collectibles::shades, 10, 30, 25, 1.0f, vZero, 0.1f, 1.5f, babyShadeTreeColor, shadeTreeColor, deadShadeTreeColor, 0.2f, 3.0f, 1, 1, "Shade tree");
	
		RGBA babyBowlerTreeColor = RGBA(111, 101, 143), bowlerTreeColor = RGBA(21, 0, 89), deadBowlerTree = RGBA(12, 4, 36);
		Tree* bowlerTree = new Tree(Collectibles::bowler, 1, 50, 50, 16.0f, vZero, 0.3f, 2.5f, babyBowlerTreeColor, bowlerTreeColor, deadBowlerTree, 1, 5, 50, 50, "Bowler tree");
	
		RGBA babyVacuumiumTreeColor = RGBA(242, 239, 148), vacuumiumTreeColor = RGBA(204, 202, 153), deadVacuumiumTreeColor = RGBA(158, 156, 85);
		Tree* vacuumiumTree = new Tree(Collectibles::vacuumium, 10, 60, 5, 0.5f, vZero, 4, 0.25f, babyVacuumiumTreeColor, vacuumiumTreeColor, deadVacuumiumTreeColor, 0.2f, 3.0f, 6, 6, "Vacuumium tree");
	}


	namespace Vines
	{
		RGBA babyCheeseVineColor = RGBA(255, 210, 112), cheeseVineColor = RGBA(200, 160, 75), deadCheeseVineColor = RGBA(140, 110, 50);
		Vine* cheeseVine = new Vine(Collectibles::cheese, 5, 10, 3, 7, 2.0f, vZero, 0.5f, babyCheeseVineColor, cheeseVineColor, deadCheeseVineColor, 1, 1, 1, "Cheese vine");

		RGBA babyLeadVineColor = RGBA(198, 111, 227), leadVineColor = RGBA(153, 29, 194), deadLeadVineColor = RGBA(15, 50, 61);
		Vine* leadVine = new Vine(Collectibles::lead, 2, 6, 4, 2, 6.0f, vZero, 0.5f, babyLeadVineColor, leadVineColor, deadLeadVineColor, 2, 1, 1, "Lead vine");
		
		RGBA babyTopazVineColor = RGBA(255, 218, 84), topazVineColor = RGBA(181, 142, 0), deadTopazVineColor = RGBA(107, 84, 0);
		Vine* topazVine = new Vine(Collectibles::topaz, 2, 5, 7, 5, 12.0f, vZero, 1.5f, babyTopazVineColor, topazVineColor, deadTopazVineColor, 5, 6, 6, "Topaz vine");
		
		RGBA babySapphireVineColor = RGBA(125, 91, 212), sapphireVineColor = RGBA(132, 89, 255), deadSapphireVineColor = RGBA(75, 69, 92);
		Vine* sapphireVine = new Vine(Collectibles::sapphire->Clone(5), 10, 13, 4, 5, 0.0625f, vZero, 0.5f, babySapphireVineColor, sapphireVineColor, deadSapphireVineColor, 1, 6, 6, "Sapphire vine");
	}

	// Keep a list of all of the plants. Tree is the base of all plants so it's what we'll use for the pointer.
	vector<Tree*> plants{ Trees::copperTree, Trees::ironTree,
		Trees::rubyTree, Trees::emeraldTree, Trees::rockTree, Trees::shadeTree, Trees::bowlerTree, Trees::vacuumiumTree,
		Vines::cheeseVine, Vines::topazVine, Vines::sapphireVine, Vines::leadVine };
}

namespace Resources::Seeds
{
	// Trees
	PlacedOnLanding* copperTreeSeed = new PlacedOnLanding(Plants::Trees::copperTree, "Copper tree seed", "Seed", 4, Plants::Trees::copperTreeColor, 0, 1, 15, false, 0.25f, Plants::Trees::copperTree->radius);
	PlacedOnLanding* ironTreeSeed = new PlacedOnLanding(Plants::Trees::ironTree, "Iron tree seed", "Seed", 4, Plants::Trees::ironTreeColor, 0, 1, 15, false, 0.25f, Plants::Trees::ironTree->radius);
	PlacedOnLanding* rockTreeSeed = new PlacedOnLanding(Plants::Trees::rockTree, "Rock tree seed", "Seed", 4, Plants::Trees::rockTreeColor, 0, 1, 15, false, 0.25f, Plants::Trees::rockTree->radius);
	CorruptOnKill* rubyTreeSeed = new CorruptOnKill(Plants::Trees::rubyTree, "Ruby tree seed", "Corruption Seed", 2, Plants::Trees::rubyTreeColor, 1, 1, 15, false, 0.25f, Plants::Trees::rubyTree->radius);
	CorruptOnKill* emeraldTreeSeed = new CorruptOnKill(Plants::Trees::emeraldTree, "Emerald tree seed", "Corruption Seed", 2, Plants::Trees::emeraldTreeColor, 1, 1, 15, false, 0.25f, Plants::Trees::emeraldTree->radius);
	PlacedOnLanding* shadeTreeSeed = new PlacedOnLanding(Plants::Trees::shadeTree, "Shade tree seed", "Seed", 4, Plants::Trees::shadeTreeColor, 0, 1, 15, false, 0.25f, Plants::Trees::shadeTree->radius);
	PlacedOnLanding* bowlerTreeSeed = new PlacedOnLanding(Plants::Trees::bowlerTree, "Bowler tree seed", "Seed", 4, Plants::Trees::bowlerTreeColor, 0, 1, 15, false, 0.5f, Plants::Trees::bowlerTree->radius);
	PlacedOnLanding* vacuumiumTreeSeed = new PlacedOnLanding(Plants::Trees::vacuumiumTree, "Vacuumium tree seed", "Seed", 4, Plants::Trees::vacuumiumTreeColor, 0, 1, 15, false, 0.5f, Plants::Trees::vacuumiumTree->radius);
	// Vines
	PlacedOnLanding* cheeseVineSeed = new PlacedOnLanding(Plants::Vines::cheeseVine, "Cheese vine seed", "Seed", 4, Plants::Vines::cheeseVineColor, 0);
	PlacedOnLanding* topazTreeSeed = new PlacedOnLanding(Plants::Vines::topazVine, "Topaz vine seed", "Seed", 4, Plants::Vines::topazVineColor, 0, 1, 15.0f, false, 0.25f, 1.5f);
	CorruptOnKill* sapphireTreeSeed = new CorruptOnKill(Plants::Vines::sapphireVine, "Sapphire vine seed", "Corruption Seed", 2, Plants::Vines::sapphireVineColor, 1);
	PlacedOnLanding* leadVineSeed = new PlacedOnLanding(Plants::Vines::leadVine, "Lead vine seed", "Seed", 4, Plants::Vines::leadVineColor, 0);

	// Keep a list of all of the seeds.
	vector<Item*> plantSeeds{ copperTreeSeed, ironTreeSeed, rubyTreeSeed, emeraldTreeSeed, rockTreeSeed, shadeTreeSeed, bowlerTreeSeed, vacuumiumTreeSeed, cheeseVineSeed, topazTreeSeed, sapphireTreeSeed, leadVineSeed };
}

namespace Collectibles::Seeds
{
	// Vines
	Collectible* copperTreeSeed = new Collectible(*Resources::Seeds::copperTreeSeed);
	Collectible* ironTreeSeed = new Collectible(*Resources::Seeds::ironTreeSeed);
	Collectible* rubyTreeSeed = new Collectible(*Resources::Seeds::rubyTreeSeed);
	Collectible* emeraldTreeSeed = new Collectible(*Resources::Seeds::emeraldTreeSeed);
	Collectible* rockTreeSeed = new Collectible(*Resources::Seeds::rockTreeSeed);
	Collectible* shadeTreeSeed = new Collectible(*Resources::Seeds::shadeTreeSeed);
	Collectible* bowlerTreeSeed = new Collectible(*Resources::Seeds::bowlerTreeSeed);
	Collectible* vacuumiumTreeSeed = new Collectible(*Resources::Seeds::vacuumiumTreeSeed);
	// Vines
	Collectible* cheeseVineSeed = new Collectible(*Resources::Seeds::cheeseVineSeed);
	Collectible* topazTreeSeed = new Collectible(*Resources::Seeds::topazTreeSeed);
	Collectible* sapphireTreeSeed = new Collectible(*Resources::Seeds::sapphireTreeSeed);
	Collectible* leadVindSeed = new Collectible(*Resources::Seeds::leadVineSeed);

	// Keep a list of all of the seeds.
	vector<Collectible*> plantSeeds{ copperTreeSeed, ironTreeSeed, rubyTreeSeed, emeraldTreeSeed, rockTreeSeed, shadeTreeSeed, bowlerTreeSeed,
		vacuumiumTreeSeed, cheeseVineSeed, topazTreeSeed, sapphireTreeSeed, leadVindSeed };
}

#pragma endregion