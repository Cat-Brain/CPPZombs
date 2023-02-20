#include "BuildingBlocks.h"


float difficultyGrowthModifier[] = { 2.0f, 1.0f, 0.5f };

class Tree : public FunctionalBlock2
{
public:
	Collectible* collectible, *seed;
	RGBA adultColor, deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;

	Tree(Collectible* collectible, int cyclesToGrow, int deadStage, int chanceForSeed,
		float timePer, Vec2 pos = vZero, Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA adultColor = RGBA(),
		RGBA deadColor = RGBA(), RGBA subsurfaceResistance = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(nullptr), cyclesToGrow(cyclesToGrow), deadStage(deadStage),
		currentLifespan(0), chanceForSeed(chanceForSeed), adultColor(adultColor), deadColor(deadColor),
		FunctionalBlock2(timePer, pos, dimensions, color, subsurfaceResistance, mass, maxHealth, health, name)
	{
		dUpdate = DUPDATE::TREEDU;
		uiUpdate = UIUPDATE::TREEUIU;
		tUpdate = TUPDATE::TREETU;
	}

	void Start() override
	{
		FunctionalBlock2::Start();
		timeSince = timePer * RandFloat();
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
		if (tree->currentLifespan >= tree->deadStage)
			tree->color = tree->deadColor;
		else if (tree->currentLifespan >= tree->cyclesToGrow)
			tree->color = tree->adultColor;
		tree->DUpdate(DUPDATE::ENTITYDU);
	}
}


class Vine : public Tree
{
public:
	int maxGenerations, generation;

	Vine(Collectible* collectible, int cyclesToGrow, int deadStage, int maxGenerations, int chanceForSeed,
		float timePer, Vec2 pos = vZero, Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA adultColor = RGBA(),
		RGBA deadColor = RGBA(), RGBA subsurfaceResistance = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Tree(collectible, cyclesToGrow, deadStage, chanceForSeed, timePer, pos, dimensions, color, adultColor, deadColor, subsurfaceResistance, mass, maxHealth, health, name),
		maxGenerations(maxGenerations), generation(0)
	{
		uiUpdate = UIUPDATE::VINEUIU;
		tUpdate = TUPDATE::VINETU;
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

	void OnDeath(Entity* damageDealer) override
	{
		Tree::OnDeath(damageDealer);
		if (rand() % 100 < chanceForSeed)
			game->entities->push_back(seed->Clone(pos));
		else
			game->entities->push_back(collectible->Clone(pos));
	}
};


namespace TUpdates
{
	bool TreeTU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);
		if (tree->currentLifespan >= tree->cyclesToGrow && tree->currentLifespan < tree->deadStage)
		{
			if (rand() % 100 < tree->chanceForSeed)
				game->entities->push_back(tree->seed->Clone(tree->pos + (tree->dimensions + tree->seed->dimensions) / 2 * Vec2((rand() % 2) * 2 - 1, (rand() % 2) * 2 - 1)));
			else
				game->entities->push_back(tree->collectible->Clone(tree->pos + (tree->dimensions + tree->collectible->dimensions) / 2 * Vec2((rand() % 2) * 2 - 1, (rand() % 2) * 2 - 1)));
		}
		tree->currentLifespan++;
		return true;
	}

	bool VineTU(Entity* entity)
	{
		Vine* vine = static_cast<Vine*>(entity);
		if (vine->generation < vine->maxGenerations && vine->currentLifespan >= vine->cyclesToGrow && vine->currentLifespan < vine->deadStage)
		{
			Vec2 placementPos = vine->pos;
			while (placementPos == vine->pos)
				placementPos = vine->pos + vine->dimensions * Vec2((rand() % 3) - 1, (rand() % 3) - 1);
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(placementPos, vine->dimensions);
			if (!hitEntities.size())
			{
				unique_ptr<Entity> newVine = vine->baseClass->Clone(placementPos, up, vine);
				((Vine*)newVine.get())->generation = vine->generation + 1;
				game->entities->push_back(std::move(newVine));
			}
		}
		vine->currentLifespan++;
		return true;
	}
}


namespace UIUpdates
{
	void TreeUIU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);

		Vec2f bottomLeft = tree->BottomLeft();
		if (tree->currentLifespan < tree->cyclesToGrow)
		{
			tree->DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Baby " + tree->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp / 2),
				COMMON_BOARDER_WIDTH, "Baby " + tree->name, tree->color, tree->deadColor, tree->collectible->color);
			font.Render(ToStringWithPrecision(tree->timePer * (tree->cyclesToGrow - tree->currentLifespan) - tree->timeSince, 1), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), tree->color);
		}
		else if (tree->currentLifespan < tree->deadStage)
		{
			tree->DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Adult " + tree->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp * 3 / 4),
				COMMON_BOARDER_WIDTH, "Adult " + tree->name, tree->color, tree->deadColor, tree->collectible->color);
			font.Render(ToStringWithPrecision(tree->timePer - tree->timeSince, 1), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), tree->color);
			font.Render(ToStringWithPrecision(tree->timePer * (tree->deadStage - tree->currentLifespan) - tree->timeSince, 1), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), tree->color);
		}
		else
			tree->DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Dead " + tree->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp / 4),
				COMMON_BOARDER_WIDTH, "Dead " + tree->name, tree->deadColor, tree->color, tree->collectible->color);
	}

	void VineUIU(Entity* entity)
	{
		Vine* vine = static_cast<Vine*>(entity);
		Vec2f bottomLeft = vine->BottomLeft();
		if (vine->currentLifespan < vine->cyclesToGrow)
		{
			vine->DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Baby " + vine->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp * 3 / 4),
				COMMON_BOARDER_WIDTH, "Baby " + vine->name, vine->color, vine->deadColor, vine->collectible->color);
			font.Render(ToStringWithPrecision(vine->timePer * (vine->cyclesToGrow - vine->currentLifespan) - vine->timeSince, 1), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
			font.Render("Gen " + to_string(vine->generation), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
		}
		else if (vine->currentLifespan < vine->deadStage)
		{
			vine->DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Adult " + vine->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp),
				COMMON_BOARDER_WIDTH, "Adult " + vine->name, vine->color, vine->deadColor, vine->collectible->color);
			font.Render(ToStringWithPrecision(vine->timePer - vine->timeSince, 1), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
			font.Render(ToStringWithPrecision(vine->timePer * (vine->deadStage - vine->currentLifespan) - vine->timeSince, 1), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
			font.Render("Gen " + to_string(vine->generation), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp * 3 / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
		}
		else
		{
			vine->DrawUIBox(bottomLeft, bottomLeft + Vec2(font.TextWidth("Dead " + vine->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp / 2),
				COMMON_BOARDER_WIDTH, "Dead " + vine->name, vine->deadColor, vine->color, vine->collectible->color);
			font.Render("Gen " + to_string(vine->generation), bottomLeft +
				Vec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), vine->color);
		}
	}
}


#pragma region Plants

namespace Plants
{
	namespace Trees
	{
		RGBA babyCopperTreeColor = RGBA(207, 137, 81), copperTreeColor = RGBA(163, 78, 8), deadCopperTreeColor = RGBA(94, 52, 17), copperResistence = RGBA(0, 0, 50);
		Tree* copperTree = new Tree(Collectibles::copper, 5, 25, 25, 4.0f, vZero, vOne, babyCopperTreeColor, copperTreeColor, deadCopperTreeColor, copperResistence, 1, 1, 1, "Copper tree");

		RGBA babyIronTreeColor = RGBA(96, 192, 225), ironTreeColor = RGBA(67, 90, 99), deadIronTreeColor = RGBA(45, 47, 48), ironResistence = RGBA(50, 50, 0);
		Tree* ironTree = new Tree(Collectibles::iron, 120, 180, 10, 0.5f, vZero, vOne, babyIronTreeColor, ironTreeColor, deadIronTreeColor, ironResistence, 1, 1, 1, "Iron tree");

		RGBA babyRubyTreeColor = RGBA(207, 120, 156), rubyTreeColor = RGBA(135, 16, 66), deadRubyTreeColor = RGBA(120, 65, 88), rubyResistence = RGBA(0, 50, 50);
		Tree* rubyTree = new Tree(Collectibles::ruby, 5, 15, 50, 4.0f, vZero, vOne, babyRubyTreeColor, rubyTreeColor, deadRubyTreeColor, rubyResistence, 1, 1, 1, "Ruby tree");

		RGBA babyEmeraldTreeColor = RGBA(145, 255, 204), emeraldTreeColor = RGBA(65, 166, 119), deadEmeraldTreeColor = RGBA(61, 97, 80), emeraldResistence = RGBA(50, 0, 50);
		Tree* emeraldTree = new Tree(Collectibles::emerald, 5, 15, 50, 4.0f, vZero, vOne, babyEmeraldTreeColor, emeraldTreeColor, deadEmeraldTreeColor, emeraldResistence, 1, 1, 1, "Emerald tree");

		RGBA babyRockTreeColor = RGBA(212, 212, 212), rockTreeColor = RGBA(201, 196, 165), deadRockTreeColor = RGBA(130, 130, 130), rockResistence = RGBA(50, 0, 50);
		Tree* rockTree = new Tree(Collectibles::rock, 5, 8, 75, 4.0f, vZero, vOne, babyRockTreeColor, rockTreeColor, deadRockTreeColor, rockResistence, 1, 1, 1, "Rock tree");
		
		RGBA babyShadeTreeColor = RGBA(50, 50), shadeTreeColor = RGBA(25, 25), deadShadeTreeColor = RGBA(), shadeResistence = RGBA();
		Tree* shadeTree = new Tree(Collectibles::shades, 10, 30, 25, 1.0f, vZero, vOne, babyShadeTreeColor, shadeTreeColor, deadShadeTreeColor, shadeResistence, 1, 1, 1, "Shade tree");
	
		RGBA babyBowlerTreeColor = RGBA(111, 101, 143), bowlerTreeColor = RGBA(21, 0, 89), deadBowlerTree = RGBA(12, 4, 36), bowlerResistence = RGBA(5, 0, 10);
		Tree* bowlerTree = new Tree(Collectibles::bowler, 1, 50, 50, 16.0f, vZero, vOne * 3, babyBowlerTreeColor, bowlerTreeColor, deadBowlerTree, bowlerResistence, 5, 50, 50, "Bowler tree");
	}


	namespace Vines
	{
		RGBA babyCheeseVineColor = RGBA(255, 210, 112), cheeseVineColor = RGBA(200, 160, 75), deadCheeseVineColor = RGBA(140, 110, 50), cheeseResistence = RGBA(0, 0, 50);
		Vine* cheeseVine = new Vine(Collectibles::cheese, 5, 10, 5, 10, 2.0f, vZero, vOne, babyCheeseVineColor, cheeseVineColor, deadCheeseVineColor, cheeseResistence, 1, 1, 1, "Cheese vine");

		RGBA babyLeadVineColor = RGBA(198, 111, 227), leadVineColor = RGBA(153, 29, 194), deadLeadVineColor = RGBA(15, 50, 61), leadResistence = RGBA(0, 50, 0);
		Vine* leadVine = new Vine(Collectibles::lead, 2, 6, 15, 5, 3.0f, vZero, vOne, babyLeadVineColor, leadVineColor, deadLeadVineColor, leadResistence, 2, 1, 1, "Lead vine");
		
		RGBA babyTopazVineColor = RGBA(255, 218, 84), topazVineColor = RGBA(181, 142, 0), deadTopazVineColor = RGBA(107, 84, 0), topazResistence = RGBA(25, 0, 50);
		Vine* topazVine = new Vine(Collectibles::topaz, 2, 5, 30, 5, 4.0f, vZero, vOne * 3, babyTopazVineColor, topazVineColor, deadTopazVineColor, topazResistence, 5, 6, 6, "Topaz vine");
		
		RGBA babySapphireVineColor = RGBA(125, 91, 212), sapphireVineColor = RGBA(132, 89, 255), deadSapphireVineColor = RGBA(75, 69, 92), sapphireResistence = RGBA(50, 50, 0);
		Vine* sapphireVine = new Vine(Collectibles::sapphire->Clone(5), 2, 4, 30, 5, 0.25f, vZero, vOne, babySapphireVineColor, sapphireVineColor, deadSapphireVineColor, sapphireResistence, 1, 6, 6, "Sapphire vine");
	}

	// Keep a list of all of the plants. Tree is the base of all plants so it's what we'll use for the pointer.
	vector<Tree*> plants{ Trees::copperTree, Trees::ironTree,
		Trees::rubyTree, Trees::emeraldTree, Trees::rockTree, Trees::shadeTree, Trees::bowlerTree,
		Vines::cheeseVine, Vines::topazVine, Vines::sapphireVine, Vines::leadVine };
}

namespace Resources::Seeds
{
	// Trees
	PlacedOnLanding* copperTreeSeed = new PlacedOnLanding(Plants::Trees::copperTree, "Copper tree seed", "Seed", 4, Plants::Trees::copperTreeColor, Resources::copper->subScat, 0);
	PlacedOnLanding* ironTreeSeed = new PlacedOnLanding(Plants::Trees::ironTree, "Iron tree seed", "Seed", 4, Plants::Trees::ironTreeColor, Resources::iron->subScat, 0);
	PlacedOnLanding* rockTreeSeed = new PlacedOnLanding(Plants::Trees::rockTree, "Rock tree seed", "Seed", 4, Plants::Trees::rockTreeColor, Resources::rock->subScat, 0);
	CorruptOnKill* rubyTreeSeed = new CorruptOnKill(Plants::Trees::rubyTree, "Ruby tree seed", "Corruption Seed", 2, Plants::Trees::rubyTreeColor, Resources::ruby->subScat, 1);
	CorruptOnKill* emeraldTreeSeed = new CorruptOnKill(Plants::Trees::emeraldTree, "Emerald tree seed", "Corruption Seed", 2, Plants::Trees::emeraldTreeColor, Resources::emerald->subScat, 1);
	PlacedOnLanding* shadeTreeSeed = new PlacedOnLanding(Plants::Trees::shadeTree, "Shade tree seed", "Seed", 4, Plants::Trees::shadeTreeColor, Resources::shades->subScat, 0);
	PlacedOnLanding* bowlerTreeSeed = new PlacedOnLanding(Plants::Trees::bowlerTree, "Bowler tree seed", "Seed", 4, Plants::Trees::bowlerTreeColor, Resources::bowler->subScat, 0, 1, 15, false, 0.25f, vOne * 3);
	// Vines
	PlacedOnLanding* cheeseVineSeed = new PlacedOnLanding(Plants::Vines::cheeseVine, "Cheese vine seed", "Seed", 4, Plants::Vines::cheeseVineColor, Resources::cheese->subScat, 0);
	PlacedOnLanding* topazTreeSeed = new PlacedOnLanding(Plants::Vines::topazVine, "Topaz vine seed", "Seed", 4, Plants::Vines::topazVineColor, Resources::topaz->subScat, 0, 1, 15.0f, false, 0.25f, vOne * 3);
	CorruptOnKill* sapphireTreeSeed = new CorruptOnKill(Plants::Vines::sapphireVine, "Sapphire vine seed", "Corruption Seed", 2, Plants::Vines::sapphireVineColor, Resources::sapphire->subScat, 1);
	PlacedOnLanding* leadVineSeed = new PlacedOnLanding(Plants::Vines::leadVine, "Lead vine seed", "Seed", 4, Plants::Vines::leadVineColor, Resources::lead->subScat, 0);

	// Keep a list of all of the seeds.
	vector<Item*> plantSeeds{ copperTreeSeed, ironTreeSeed, rubyTreeSeed, emeraldTreeSeed, rockTreeSeed, shadeTreeSeed, bowlerTreeSeed, cheeseVineSeed, topazTreeSeed, sapphireTreeSeed, leadVineSeed };
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
	// Vines
	Collectible* cheeseVineSeed = new Collectible(*Resources::Seeds::cheeseVineSeed);
	Collectible* topazTreeSeed = new Collectible(*Resources::Seeds::topazTreeSeed);
	Collectible* sapphireTreeSeed = new Collectible(*Resources::Seeds::sapphireTreeSeed);
	Collectible* leadVindSeed = new Collectible(*Resources::Seeds::leadVineSeed);

	// Keep a list of all of the seeds.
	vector<Collectible*> plantSeeds{ copperTreeSeed, ironTreeSeed, rubyTreeSeed, emeraldTreeSeed, rockTreeSeed, shadeTreeSeed, bowlerTreeSeed, cheeseVineSeed, topazTreeSeed, sapphireTreeSeed, leadVindSeed };
}

#pragma endregion