#include "BuildingBlocks.h"


float difficultyGrowthModifier[] = { 2.0f, 1.0f, 0.5f };

class Tree : public FunctionalBlock2
{
public:
	ItemInstance collectible, seed;
	RGBA adultColor, deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;
	float babyRadius, maxRadius, babyMass, maxMass;
	float nextPlacementRotation = 0.0f;
	bool nextSpawnSeed = true;

	Tree(ItemInstance collectible, ItemInstance seed, int cyclesToGrow, int deadStage, int chanceForSeed,
		float timePer, float radius = 0.5f, float maxRadius = 0.5f, RGBA color = RGBA(), RGBA adultColor = RGBA(), RGBA deadColor = RGBA(),
		float mass = 1, float maxMass = 2, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(seed), cyclesToGrow(cyclesToGrow), deadStage(deadStage),
		currentLifespan(0), chanceForSeed(chanceForSeed), adultColor(adultColor), deadColor(deadColor),
		babyRadius(radius), maxRadius(maxRadius), babyMass(mass), maxMass(maxMass),
		FunctionalBlock2(timePer, pos, radius, color, mass, maxHealth, health, name)
	{
		dUpdate = DUPDATE::TREE;
		uiUpdate = UIUPDATE::TREE;
		tUpdate = TUPDATE::TREE;
	}

	Tree(Tree* baseClass, Vec3 dir, Vec3 pos) :
		Tree(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Tree>(this, dir, pos);
	}

	virtual float AreaQuality()
	{
		TILE tile = TILE(game->entities->TileAtPos(pos - Vec3(0, 0, radius + 0.1f)));
		return game->BrightnessAtPos(pos) * difficultyGrowthModifier[game->settings.difficulty] *
			(float(tile == TILE::MAX_SOIL) + 0.75f * float(tile == TILE::MID_SOIL) + 0.5f * float(tile == TILE::BAD_SOIL) +
				RUBY_SOIL_MULTIPLIER * float(tile == TILE::RUBY_SOIL));
	}

	float TimeIncrease() override
	{
		return game->dTime * AreaQuality();
	}
};

namespace DUpdates
{
	void TreeDU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);
		if (tree->radius != tree->maxRadius)
		{
			float lerpValue = min(1.f, (tree->currentLifespan + tree->timeSince / tree->timePer) / tree->cyclesToGrow);
			tree->SetRadius(tree->babyRadius + lerpValue * (tree->maxRadius - tree->babyRadius));
			tree->mass = tree->babyMass + lerpValue * (tree->maxMass - tree->babyMass);
		}

		if (tree->currentLifespan >= tree->deadStage)
			tree->color = tree->deadColor;
		else if (tree->currentLifespan >= tree->cyclesToGrow)
		{
			ItemInstance collectible = tree->nextSpawnSeed ? tree->seed : tree->collectible;
			float collectibleRadius = collectible->radius * tree->timeSince / tree->timePer;
			game->DrawCircle(tree->pos + (tree->radius + collectibleRadius) * CircPoint(tree->nextPlacementRotation, 0.f), collectible->color, collectibleRadius);
			tree->color = tree->adultColor;
		}
		tree->DUpdate(DUPDATE::ENTITY);
	}
}


class Vine : public Tree
{
public:
	float angleWobble;
	int bifurcationChance, maxGenerations, generation;

	Vine(ItemInstance collectible, ItemInstance seed, float angleWobble, int bifurcationChance, int cyclesToGrow, int maxGenerations, int chanceForSeed,
		float timePer, float radius = 0.5f, float maxRadius = 0.5f, RGBA color = RGBA(), RGBA adultColor = RGBA(),
		RGBA deadColor = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Tree(collectible, seed, cyclesToGrow, cyclesToGrow + 1, chanceForSeed, timePer, radius, maxRadius, color, adultColor, deadColor,
			mass, mass, maxHealth, health, name),
		maxGenerations(maxGenerations), generation(0), angleWobble(angleWobble), bifurcationChance(bifurcationChance)
	{
		uiUpdate = UIUPDATE::VINE;
		onDeath = ONDEATH::VINE;
		tUpdate = TUPDATE::VINE;
	}

	void Start() override
	{
		Tree::Start();
		bool bifurcated = rand() % 100 < bifurcationChance;
		deadStage = cyclesToGrow + 1 + int(bifurcated);
		dir = glm::rotateZ(dir, -float(bifurcated) * PI_F * 0.1666f);
		nextPlacementRotation = atan2f(dir.y, dir.x);
		radius = babyRadius;
		timeSince = timePer * RandFloat();
	}

	Vine(Vine* baseClass, Vec3 dir, Vec3 pos) :
		Vine(*baseClass)
	{
		this->pos = pos;
		this->dir = glm::rotateZ(dir, (RandFloat() * 2 - 1) * angleWobble);
		this->baseClass = baseClass;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Vine>(this, dir, pos);
	}

	float AreaQuality() override
	{
		TILE tile = TILE(game->entities->TileAtPos(pos - Vec3(0, 0, radius + 0.1f)));
		return (1.0f - game->BrightnessAtPos(pos)) * difficultyGrowthModifier[game->settings.difficulty] *
			(float(tile == TILE::ROCK) + 0.75f * float(tile == TILE::SAND) + 0.5f * float(tile == TILE::BAD_SOIL) + 0.25f * float(tile == TILE::MID_SOIL));
	}
};

namespace OnDeaths
{
	void VineOD(Entity* entity, Entity* damageDealer)
	{
		Vine* vine = static_cast<Vine*>(entity);
		if (rand() % 100 < vine->chanceForSeed)
			game->entities->push_back(make_unique<Collectible>(vine->seed, vine->pos));
		else
			game->entities->push_back(make_unique<Collectible>(vine->collectible, vine->pos));
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
				game->entities->push_back(make_unique<Collectible>(tree->seed.Clone(), tree->pos + (tree->radius + tree->seed->radius) * CircPoint(tree->nextPlacementRotation, 0.f)));
			else
				game->entities->push_back(make_unique<Collectible>(tree->collectible.Clone(), tree->pos + (tree->radius + tree->collectible->radius) * CircPoint(tree->nextPlacementRotation, 0.f)));
		}
		tree->currentLifespan++;
		tree->nextPlacementRotation = RandFloat() * 2 * PI_F;
		tree->nextSpawnSeed = rand() % 100 < tree->chanceForSeed;
		return true;
	}

	bool VineTU(Entity* entity)
	{
		Vine* vine = static_cast<Vine*>(entity);
		Vec3 placementPos = vine->pos + vine->radius * 2 * CircPoint(vine->nextPlacementRotation, 0.f);
		if (vine->generation >= vine->maxGenerations || game->entities->OverlapsAny(placementPos, vine->radius - 0.01f, MaskF::IsCorporealNotCollectible, vine))
			vine->currentLifespan = vine->deadStage;
		if (vine->currentLifespan >= vine->cyclesToGrow && vine->currentLifespan < vine->deadStage)
		{
			unique_ptr<Entity> newVine = vine->baseClass->Clone(placementPos, vine->dir, vine);
			((Vine*)newVine.get())->generation = vine->generation + 1;
			game->entities->push_back(std::move(newVine));
			vine->dir = RotateBy(vine->dir, PI_F * 0.33333f);
			vine->nextPlacementRotation = atan2f(vine->dir.y, vine->dir.x);
		}
		vine->currentLifespan++;
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
// Having an enum of all seeds will come in handy:
enum class SEEDINDICES
{
	COPPER, IRON, RUBY, EMERALD, ROCK, SHADE, BOWLER, VACUUMIUM, SILVER, QUARTZ_T, CHEESE, TOPAX, SAPPHIRE, LEAD, QUARTZ_V
};


namespace Plants
{
	namespace Trees
	{
		RGBA babyCopperTreeColor = RGBA(207, 137, 81), copperTreeColor = RGBA(163, 78, 8), deadCopperTreeColor = RGBA(94, 52, 17);
		Tree* copperTree = new Tree(Resources::copper->Clone(), ItemInstance(ITEMTYPE::COPPER_TREE_SEED), 5, 25, 25, 4.0f, 0.25f, 1.5f, babyCopperTreeColor, copperTreeColor, deadCopperTreeColor, 0.2f, 3.0f, 1, 1, "Copper tree");

		RGBA babyIronTreeColor = RGBA(96, 192, 225), ironTreeColor = RGBA(67, 90, 99), deadIronTreeColor = RGBA(45, 47, 48);
		Tree* ironTree = new Tree(Resources::iron->Clone(), ItemInstance(ITEMTYPE::IRON_TREE_SEED), 120, 180, 10, 0.5f, 0.25f, 1.5f, babyIronTreeColor, ironTreeColor, deadIronTreeColor, 0.2f, 3.0f, 1, 1, "Iron tree");

		RGBA babyRubyTreeColor = RGBA(207, 120, 156), rubyTreeColor = RGBA(135, 16, 66), deadRubyTreeColor = RGBA(120, 65, 88);
		Tree* rubyTree = new Tree(Resources::ruby->Clone(), ItemInstance(ITEMTYPE::RUBY_TREE_SEED), 5, 15, 50, 4.0f, 0.25f, 1.5f, babyRubyTreeColor, rubyTreeColor, deadRubyTreeColor, 0.2f, 3.0f, 1, 1, "Ruby tree");

		RGBA babyEmeraldTreeColor = RGBA(145, 255, 204), emeraldTreeColor = RGBA(65, 166, 119), deadEmeraldTreeColor = RGBA(61, 97, 80);
		Tree* emeraldTree = new Tree(Resources::emerald->Clone(), ItemInstance(ITEMTYPE::EMERALD_TREE_SEED), 5, 15, 50, 4.0f, 0.25f, 1.5f, babyEmeraldTreeColor, emeraldTreeColor, deadEmeraldTreeColor, 0.2f, 3.0f, 1, 1, "Emerald tree");

		RGBA babyRockTreeColor = RGBA(212, 212, 212), rockTreeColor = RGBA(201, 196, 165), deadRockTreeColor = RGBA(130, 130, 130);
		Tree* rockTree = new Tree(Resources::rock->Clone(), ItemInstance(ITEMTYPE::ROCK_TREE_SEED), 5, 14, 25, 3.0f, 0.25f, 1.5f, babyRockTreeColor, rockTreeColor, deadRockTreeColor, 0.2f, 3.0f, 1, 1, "Rock tree");
		
		RGBA babyShadeTreeColor = RGBA(50, 50), shadeTreeColor = RGBA(25, 25), deadShadeTreeColor = RGBA();
		Tree* shadeTree = new Tree(Resources::shade->Clone(), ItemInstance(ITEMTYPE::SHADE_TREE_SEED), 10, 30, 25, 1.0f, 0.25f, 1.5f, babyShadeTreeColor, shadeTreeColor, deadShadeTreeColor, 0.2f, 3.0f, 1, 1, "Shade tree");
	
		RGBA babyBowlerTreeColor = RGBA(111, 101, 143), bowlerTreeColor = RGBA(21, 0, 89), deadBowlerTree = RGBA(12, 4, 36);
		Tree* bowlerTree = new Tree(Resources::bowler->Clone(), ItemInstance(ITEMTYPE::BOWLER_TREE_SEED), 1, 50, 50, 16.0f, 0.5f, 2.5f, babyBowlerTreeColor, bowlerTreeColor, deadBowlerTree, 1, 5, 50, 50, "Bowler tree");
	
		RGBA babyVacuumiumTreeColor = RGBA(242, 239, 148), vacuumiumTreeColor = RGBA(204, 202, 153), deadVacuumiumTreeColor = RGBA(158, 156, 85);
		Tree* vacuumiumTree = new Tree(Resources::vacuumium->Clone(), ItemInstance(ITEMTYPE::VACUUMIUM_TREE_SEED), 10, 60, 5, 0.5f, 2.f, 0.25f, babyVacuumiumTreeColor, vacuumiumTreeColor, deadVacuumiumTreeColor, 0.2f, 3.0f, 6, 6, "Vacuumium tree");
	
		RGBA babySilverTreeColor = RGBA(209, 157, 157), silverTreeColor = RGBA(171, 171, 171), deadSilverTreeColor = RGBA(87, 99, 74);
		Tree* silverTree = new Tree(Resources::silver->Clone(), ItemInstance(ITEMTYPE::SILVER_TREE_SEED), 4, 54, 10, 10.f, 0.125f, 1.f, babySilverTreeColor, silverTreeColor, deadSilverTreeColor, 0.5f, 1.f, 1, 1, "Silver tree");
		
		RGBA babyQuartzTreeColor = RGBA(202, 188, 224), quartzTreeColor = RGBA(161, 153, 173), deadQuartzColor = RGBA(127, 70, 212);
		Tree* quartzTree = new Tree(Resources::quartz->Clone(), ItemInstance(ITEMTYPE::QUARTZ_TREE_SEED), 3, 8, 50, 3.f, 0.25f, 0.5f, babyQuartzTreeColor, quartzTreeColor, deadQuartzColor, 0.25f, 0.5f, 1, 1, "Quartz tree");
	}


	namespace Vines
	{
		RGBA babyCheeseVineColor = RGBA(255, 210, 112), cheeseVineColor = RGBA(200, 160, 75), deadCheeseVineColor = RGBA(140, 110, 50);
		Vine* cheeseVine = new Vine(Resources::cheese->Clone(), ItemInstance(ITEMTYPE::CHEESE_VINE_SEED), 0.0f, 25, 1, 25, 25, 2.0f, 0.25f, 0.5f, babyCheeseVineColor, cheeseVineColor, deadCheeseVineColor, 1, 1, 1, "Cheese vine");

		RGBA babyLeadVineColor = RGBA(198, 111, 227), leadVineColor = RGBA(153, 29, 194), deadLeadVineColor = RGBA(15, 50, 61);
		Vine* leadVine = new Vine(Resources::lead->Clone(), ItemInstance(ITEMTYPE::LEAD_VINE_SEED), 0.8f, 25, 1, 25, 2, 3.0f, 0.25f, 0.5f, babyLeadVineColor, leadVineColor, deadLeadVineColor, 2, 1, 1, "Lead vine");
		
		RGBA babyTopazVineColor = RGBA(255, 218, 84), topazVineColor = RGBA(181, 142, 0), deadTopazVineColor = RGBA(107, 84, 0);
		Vine* topazVine = new Vine(Resources::topaz->Clone(2), ItemInstance(ITEMTYPE::TOPAZ_VINE_SEED), 0.8f, 25, 1, 50, 5, 2.0f, 0.5f, 1.5f, babyTopazVineColor, topazVineColor, deadTopazVineColor, 5, 6, 6, "Topaz vine");
		
		RGBA babySapphireVineColor = RGBA(125, 91, 212), sapphireVineColor = RGBA(132, 89, 255), deadSapphireVineColor = RGBA(75, 69, 92);
		Vine* sapphireVine = new Vine(Resources::sapphire->Clone(5), ItemInstance(ITEMTYPE::SAPPHIRE_VINE_SEED), 0.8f, 25, 3, 25, 15, 0.125f, 0.25f, 0.5f, babySapphireVineColor, sapphireVineColor, deadSapphireVineColor, 1, 4, 4, "Sapphire vine");
		
		RGBA babyQuartzVineColor = RGBA(202, 188, 224), quartzVineColor = RGBA(161, 153, 173), deadQuartzColor = RGBA(127, 70, 212);
		Vine* quartzVine = new Vine(Resources::quartz->Clone(3), ItemInstance(ITEMTYPE::QUARTZ_VINE_SEED), 1.6f, 50, 3, 10, 25, 0.25f, 0.25f, 0.5f, babyQuartzVineColor, quartzVineColor, deadQuartzColor, 1, 1, 1, "Quarts vine");
	}

	// Keep a list of all of the plants. Tree is the base of all plants so it's what we'll use for the pointer.
	vector<Tree*> plants{ Trees::copperTree, Trees::ironTree, Trees::rubyTree, Trees::emeraldTree, Trees::rockTree, Trees::shadeTree,
		Trees::bowlerTree, Trees::vacuumiumTree, Trees::silverTree, Trees::quartzTree,
		Vines::cheeseVine, Vines::topazVine, Vines::sapphireVine, Vines::leadVine, Vines::quartzVine };
}

namespace Resources::Seeds
{
	// Trees
	PlacedOnLanding* copperTreeSeed = new PlacedOnLanding(ITEMTYPE::COPPER_TREE_SEED, Plants::Trees::copperTree, "Copper tree seed", "Seed", 4, Plants::Trees::copperTreeColor, 0, 15, false, 0.25f, 12.f, Plants::Trees::copperTree->babyRadius);
	PlacedOnLanding* ironTreeSeed = new PlacedOnLanding(ITEMTYPE::IRON_TREE_SEED, Plants::Trees::ironTree, "Iron tree seed", "Seed", 4, Plants::Trees::ironTreeColor, 0, 15, false, 0.25f, 12.f, Plants::Trees::ironTree->babyRadius);
	PlacedOnLanding* rockTreeSeed = new PlacedOnLanding(ITEMTYPE::ROCK_TREE_SEED, Plants::Trees::rockTree, "Rock tree seed", "Seed", 4, Plants::Trees::rockTreeColor, 0, 15, false, 0.25f, 12.f, Plants::Trees::rockTree->babyRadius);
	CorruptOnKill* rubyTreeSeed = new CorruptOnKill(ITEMTYPE::RUBY_TREE_SEED, Plants::Trees::rubyTree, "Ruby tree seed", "Corruption Seed", 2, Plants::Trees::rubyTreeColor, 1, 15, false, 0.25f, 12.f, Plants::Trees::rubyTree->babyRadius);
	CorruptOnKill* emeraldTreeSeed = new CorruptOnKill(ITEMTYPE::EMERALD_TREE_SEED, Plants::Trees::emeraldTree, "Emerald tree seed", "Corruption Seed", 2, Plants::Trees::emeraldTreeColor, 1, 15, false, 0.25f, 12.f, Plants::Trees::emeraldTree->babyRadius);
	PlacedOnLanding* shadeTreeSeed = new PlacedOnLanding(ITEMTYPE::SHADE_TREE_SEED, Plants::Trees::shadeTree, "Shade tree seed", "Seed", 4, Plants::Trees::shadeTreeColor, 0, 15, false, 0.25f, 12.f, Plants::Trees::shadeTree->babyRadius);
	PlacedOnLanding* bowlerTreeSeed = new PlacedOnLanding(ITEMTYPE::BOWLER_TREE_SEED, Plants::Trees::bowlerTree, "Bowler tree seed", "Seed", 4, Plants::Trees::bowlerTreeColor, 0, 15, false, 0.5f, 12.f, Plants::Trees::bowlerTree->babyRadius);
	PlacedOnLanding* vacuumiumTreeSeed = new PlacedOnLanding(ITEMTYPE::VACUUMIUM_TREE_SEED, Plants::Trees::vacuumiumTree, "Vacuumium tree seed", "Seed", 4, Plants::Trees::vacuumiumTreeColor, 0, 15, false, 0.5f, 12.f, Plants::Trees::vacuumiumTree->babyRadius);
	PlacedOnLanding* silverTreeSeed = new PlacedOnLanding(ITEMTYPE::SILVER_TREE_SEED, Plants::Trees::silverTree, "Silver tree seed", "Seed", 4, Plants::Trees::silverTreeColor, 0, 15, false, 0.25f, 12.f, Plants::Trees::silverTree->babyRadius);
	PlacedOnLanding* quartzTreeSeed = new PlacedOnLanding(ITEMTYPE::QUARTZ_TREE_SEED, Plants::Trees::quartzTree, "Quartz tree seed", "Seed", 4, Plants::Trees::quartzTreeColor, 0, 15, false, 0.25f, 12.f, Plants::Trees::quartzTree->babyRadius);
	// Vines
	PlacedOnLanding* cheeseVineSeed = new PlacedOnLanding(ITEMTYPE::CHEESE_VINE_SEED, Plants::Vines::cheeseVine, "Cheese vine seed", "Seed", 4, Plants::Vines::cheeseVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::cheeseVine->babyRadius);
	PlacedOnLanding* topazVineSeed = new PlacedOnLanding(ITEMTYPE::TOPAZ_VINE_SEED, Plants::Vines::topazVine, "Topaz vine seed", "Seed", 4, Plants::Vines::topazVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::topazVine->babyRadius);
	CorruptOnKill* sapphireVineSeed = new CorruptOnKill(ITEMTYPE::SAPPHIRE_VINE_SEED, Plants::Vines::sapphireVine, "Sapphire vine seed", "Corruption Seed", 2, Plants::Vines::sapphireVineColor, 1, 15.f, false, 0.25f, 12.f, Plants::Vines::sapphireVine->babyRadius);
	PlacedOnLanding* leadVineSeed = new PlacedOnLanding(ITEMTYPE::LEAD_VINE_SEED, Plants::Vines::leadVine, "Lead vine seed", "Seed", 4, Plants::Vines::leadVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::leadVine->babyRadius);
	PlacedOnLanding* quartzVineSeed = new PlacedOnLanding(ITEMTYPE::QUARTZ_VINE_SEED, Plants::Vines::quartzVine, "Quartz vine seed", "Seed", 4, Plants::Vines::quartzVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::quartzVine->babyRadius);

	// Keep a list of all of the seeds.
	vector<Item*> plantSeeds{ copperTreeSeed, ironTreeSeed, rubyTreeSeed, emeraldTreeSeed, rockTreeSeed, shadeTreeSeed, bowlerTreeSeed,
		vacuumiumTreeSeed, silverTreeSeed, quartzTreeSeed, cheeseVineSeed, topazVineSeed, sapphireVineSeed, leadVineSeed, quartzVineSeed };
	
}

namespace Collectibles::Seeds
{
	// Vines
	Collectible* copperTreeSeed = new Collectible(Resources::Seeds::copperTreeSeed->Clone());
	Collectible* ironTreeSeed = new Collectible(Resources::Seeds::ironTreeSeed->Clone());
	Collectible* rubyTreeSeed = new Collectible(Resources::Seeds::rubyTreeSeed->Clone());
	Collectible* emeraldTreeSeed = new Collectible(Resources::Seeds::emeraldTreeSeed->Clone());
	Collectible* rockTreeSeed = new Collectible(Resources::Seeds::rockTreeSeed->Clone());
	Collectible* shadeTreeSeed = new Collectible(Resources::Seeds::shadeTreeSeed->Clone());
	Collectible* bowlerTreeSeed = new Collectible(Resources::Seeds::bowlerTreeSeed->Clone());
	Collectible* vacuumiumTreeSeed = new Collectible(Resources::Seeds::vacuumiumTreeSeed->Clone());
	Collectible* silverTreeSeed = new Collectible(Resources::Seeds::silverTreeSeed->Clone());
	Collectible* quartzTreeSeed = new Collectible(Resources::Seeds::quartzTreeSeed->Clone());
	// Vines
	Collectible* cheeseVineSeed = new Collectible(Resources::Seeds::cheeseVineSeed->Clone());
	Collectible* topazVineSeed = new Collectible(Resources::Seeds::topazVineSeed->Clone());
	Collectible* sapphireVineSeed = new Collectible(Resources::Seeds::sapphireVineSeed->Clone());
	Collectible* leadVineSeed = new Collectible(Resources::Seeds::leadVineSeed->Clone());
	Collectible* quartzVineSeed = new Collectible(Resources::Seeds::quartzVineSeed->Clone());

	// Keep a list of all of the seeds.
	vector<Collectible*> plantSeeds{ copperTreeSeed, ironTreeSeed, rubyTreeSeed, emeraldTreeSeed, rockTreeSeed, shadeTreeSeed, bowlerTreeSeed,
		vacuumiumTreeSeed, silverTreeSeed, quartzTreeSeed, cheeseVineSeed, topazVineSeed, sapphireVineSeed, leadVineSeed, quartzVineSeed };
}

#pragma endregion