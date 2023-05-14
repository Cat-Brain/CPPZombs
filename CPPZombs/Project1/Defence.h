#include "BuildingBlocks.h"


float difficultyGrowthModifier[] = { 2.0f, 1.0f, 0.5f };

class Shrub : public FunctionalBlock2
{
public:
	ItemInstance collectible, seed;
	RGBA adultColor, deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;
	float babyRadius, maxRadius, babyMass, maxMass;
	Vec3 nextPlacementPos = vZero;
	bool nextSpawnSeed = true;

	Shrub(ItemInstance collectible, ItemInstance seed, int cyclesToGrow, int deadStage, int chanceForSeed,
		float timePer, float radius = 0.5f, float maxRadius = 0.5f, RGBA color = RGBA(), RGBA adultColor = RGBA(), RGBA deadColor = RGBA(),
		float mass = 1, float maxMass = 2, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(seed), cyclesToGrow(cyclesToGrow), deadStage(deadStage),
		currentLifespan(0), chanceForSeed(chanceForSeed), adultColor(adultColor), deadColor(deadColor),
		babyRadius(radius), maxRadius(maxRadius), babyMass(mass), maxMass(maxMass),
		FunctionalBlock2(timePer, pos, radius, color, mass, maxHealth, health, name)
	{
		dUpdate = DUPDATE::SHRUB;
		uiUpdate = UIUPDATE::SHRUB;
		tUpdate = TUPDATE::SHRUB;
	}

	Shrub(Shrub* baseClass, Vec3 dir, Vec3 pos) :
		Shrub(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Shrub>(this, dir, pos);
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

class Tree : public FunctionalBlock2
{
public:
	Tree* trunk, *last = nullptr;
	ItemInstance collectible, seed;
	RGBA leafColor; // trunkColor = this->color except in leafs which are weird.
	// currentHeight will be from 1 - height in the trunk and from height+1 - height+leafLength in the leafs.
	int height, currentHeight, leafLength, chanceForSeed, branchCount;
	// Some way to control the placement of fruits
	float leafRadius;
	Vec3 nextPlacementPos = vZero;
	bool nextSpawnSeed = true, shouldGrow = true;

	Tree(ItemInstance collectible, ItemInstance seed, int height, int leafLength, int chanceForSeed, int branchCount,
		float timePer, float radius, float leafRadius, RGBA color, RGBA leafColor,
		float mass, int maxHealth, int health, string name) :
		collectible(collectible), seed(seed),
		height(height), currentHeight(0), leafLength(leafLength), chanceForSeed(chanceForSeed), leafColor(leafColor),
		leafRadius(leafRadius),
		FunctionalBlock2(timePer, vZero, radius, color, mass, maxHealth, health, name)
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
	}

	void Start() override
	{
		currentHeight = currentHeight++;
		if (currentHeight > height) // Is it a leaf?
		{
			color = leafColor;
			radius = leafRadius;
			//nextPlacementPos = someWackyValueThat'llBeAnnoyingToFind * (radius + (currentHeight + 1 > height ? leafRadius : radius));
			
			if (currentHeight == height + leafLength)
				shouldGrow = false; // Don't grow another segment and instead grow fruits.
		}
		/*else */nextPlacementPos = up * (radius + (currentHeight + 1 > height ? leafRadius : radius));
		FunctionalBlock2::Start();
	}

	unique_ptr<Tree> TClone(Vec3 dir, Vec3 pos)
	{
		unique_ptr<Tree> tree = make_unique<Tree>(this, dir, pos);
		tree->last = this;
		tree->trunk = trunk;
		tree->Start();
		return std::move(tree);
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
		return game->dTime * trunk->AreaQuality();
	}
};

namespace DUpdates
{
	void ShrubDU(Entity* entity)
	{
		Shrub* shrub = static_cast<Shrub*>(entity);
		if (shrub->radius != shrub->maxRadius)
		{
			float lerpValue = min(1.f, (shrub->currentLifespan + shrub->timeSince / shrub->timePer) / shrub->cyclesToGrow);
			shrub->SetRadius(shrub->babyRadius + lerpValue * (shrub->maxRadius - shrub->babyRadius));
			shrub->mass = shrub->babyMass + lerpValue * (shrub->maxMass - shrub->babyMass);
		}

		if (shrub->currentLifespan >= shrub->deadStage)
			shrub->color = shrub->deadColor;
		else if (shrub->currentLifespan >= shrub->cyclesToGrow)
		{
			ItemInstance collectible = shrub->nextSpawnSeed ? shrub->seed : shrub->collectible;
			float collectibleRadius = collectible->radius * shrub->timeSince / shrub->timePer;
			game->DrawCircle(shrub->pos + (shrub->radius + collectibleRadius) * shrub->nextPlacementPos, collectible->color, collectibleRadius);
			shrub->color = shrub->adultColor;
		}
		shrub->DUpdate(DUPDATE::ENTITY);
	}

	void TreeDU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);
		/*if (tree->radius != tree->baseClass->radius)
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
			game->DrawCircle(tree->pos + (tree->radius + collectibleRadius) * tree->nextPlacementPos, collectible->color, collectibleRadius);
			tree->color = tree->adultColor;
		}*/
		tree->DUpdate(DUPDATE::ENTITY);
	}
}


class Vine : public Shrub
{
public:
	float angleWobble;
	int bifurcationChance, maxGenerations, generation;

	Vine(ItemInstance collectible, ItemInstance seed, float angleWobble, int bifurcationChance, int cyclesToGrow, int maxGenerations, int chanceForSeed,
		float timePer, float radius = 0.5f, float maxRadius = 0.5f, RGBA color = RGBA(), RGBA adultColor = RGBA(),
		RGBA deadColor = RGBA(),
		float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Shrub(collectible, seed, cyclesToGrow, cyclesToGrow + 1, chanceForSeed, timePer, radius, maxRadius, color, adultColor, deadColor,
			mass, mass, maxHealth, health, name),
		maxGenerations(maxGenerations), generation(0), angleWobble(angleWobble), bifurcationChance(bifurcationChance)
	{
		uiUpdate = UIUPDATE::VINE;
		onDeath = ONDEATH::VINE;
		tUpdate = TUPDATE::VINE;
	}

	void Start() override
	{
		Shrub::Start();
		bool bifurcated = rand() % 100 < bifurcationChance;
		deadStage = cyclesToGrow + 1 + int(bifurcated);
		dir = glm::rotateZ(dir, -float(bifurcated) * PI_F * 0.1666f);
		nextPlacementPos = dir;
		radius = babyRadius;
		//timeSince = timePer * RandFloat();
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
		printf("!");
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
	bool ShrubTU(Entity* entity)
	{
		Shrub* shrub = static_cast<Shrub*>(entity);
		if (shrub->currentLifespan >= shrub->cyclesToGrow && shrub->currentLifespan < shrub->deadStage)
		{
			if (shrub->nextSpawnSeed)
				game->entities->push_back(make_unique<Collectible>(shrub->seed.Clone(), shrub->pos + (shrub->radius + shrub->seed->radius) * shrub->nextPlacementPos));
			else
				game->entities->push_back(make_unique<Collectible>(shrub->collectible.Clone(), shrub->pos + (shrub->radius + shrub->collectible->radius) * shrub->nextPlacementPos));
		}
		shrub->currentLifespan++;
		shrub->nextPlacementPos = Vec3(CircPoint2(RandFloat() * 2 * PI_F), 0);
		shrub->nextSpawnSeed = rand() % 100 < shrub->chanceForSeed;
		return true;
	}

	bool TreeTU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);
		/*if (tree->currentLifespan >= tree->cyclesToGrow && tree->currentLifespan < tree->deadStage)
		{
			if (tree->nextSpawnSeed)
				game->entities->push_back(make_unique<Collectible>(tree->seed.Clone(), tree->pos + (tree->radius + tree->seed->radius) * tree->nextPlacementPos));
			else
				game->entities->push_back(make_unique<Collectible>(tree->collectible.Clone(), tree->pos + (tree->radius + tree->collectible->radius) * tree->nextPlacementPos));
		}
		tree->currentLifespan++;
		tree->nextPlacementPos = Vec3(CircPoint2(RandFloat() * 2 * PI_F), 0);
		tree->nextSpawnSeed = rand() % 100 < tree->chanceForSeed;*/
		return true;
	}

	bool VineTU(Entity* entity)
	{
		Vine* vine = static_cast<Vine*>(entity);
		Vec3 placementPos = vine->pos + vine->radius * 2 * vine->nextPlacementPos;
		if (vine->generation >= vine->maxGenerations || game->entities->OverlapsAny(placementPos, vine->radius - 0.01f, MaskF::IsCorporealNotCollectible, vine))
			vine->currentLifespan = vine->deadStage;
		if (vine->currentLifespan >= vine->cyclesToGrow && vine->currentLifespan < vine->deadStage)
		{
			unique_ptr<Entity> newVine = vine->baseClass->Clone(placementPos, vine->dir, vine);
			((Vine*)newVine.get())->generation = vine->generation + 1;
			game->entities->push_back(std::move(newVine));
			vine->dir = RotateBy(vine->dir, PI_F * 0.33333f);
			vine->nextPlacementPos = vine->dir;
			if (game->entities->OverlapsAny(vine->pos + vine->radius * 2 * vine->nextPlacementPos, vine->radius - 0.01f, MaskF::IsCorporealNotCollectible, vine))
			{
				vine->dir = Vec3(0, 0, 1);
				vine->nextPlacementPos = vine->dir;
			}
		}
		vine->currentLifespan++;
		vine->nextSpawnSeed = true;
		return true;
	}
}


namespace UIUpdates
{
	void ShrubUIU(Entity* entity)
	{
		Shrub* shrub = static_cast<Shrub*>(entity);

		iVec2 bottomLeft = shrub->BottomLeft();
		if (shrub->currentLifespan < shrub->cyclesToGrow)
		{
			shrub->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Baby " + shrub->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp / 2),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Baby " + shrub->name, shrub->color, shrub->deadColor, shrub->collectible->color);
			font.Render(ToStringWithPrecision(shrub->timePer * (shrub->cyclesToGrow - shrub->currentLifespan) - shrub->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), shrub->color);
		}
		else if (shrub->currentLifespan < shrub->deadStage)
		{
			shrub->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Adult " + shrub->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp * 3 / 4),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Adult " + shrub->name, shrub->color, shrub->deadColor, shrub->collectible->color);
			font.Render(ToStringWithPrecision(shrub->timePer - shrub->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp / 2 - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), shrub->color);
			font.Render(ToStringWithPrecision(shrub->timePer * (shrub->deadStage - shrub->currentLifespan) - shrub->timeSince, 1), bottomLeft +
				iVec2(COMMON_BOARDER_WIDTH, font.vertDisp - font.mininumVertOffset), static_cast<float>(COMMON_TEXT_SCALE), shrub->color);
		}
		else
			shrub->DrawUIBox(bottomLeft, bottomLeft + iVec2(font.TextWidth("Dead " + shrub->name) * COMMON_TEXT_SCALE / font.minimumSize / 2, font.vertDisp / 4),
				static_cast<float>(COMMON_BOARDER_WIDTH), "Dead " + shrub->name, shrub->deadColor, shrub->color, shrub->collectible->color);
	}

	void TreeUIU(Entity* entity)
	{
		Tree* tree = static_cast<Tree*>(entity);

		iVec2 bottomLeft = tree->BottomLeft();
		/*if (tree->currentLifespan < tree->cyclesToGrow)
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
	*/}

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
	COPPER, IRON, RUBY, EMERALD, ROCK, SHADE, BOWLER, VACUUMIUM, SILVER, QUARTZ_S, CHEESE, TOPAX, SAPPHIRE, LEAD, QUARTZ_V
};


namespace Plants
{
	namespace Shrubs
	{
		RGBA babyCopperShrubColor = RGBA(207, 137, 81), copperShrubColor = RGBA(163, 78, 8), deadCopperShrubColor = RGBA(94, 52, 17);
		Shrub* copperShrub = new Shrub(Resources::copper->Clone(), ItemInstance(ITEMTYPE::COPPER_SHRUB_SEED), 5, 25, 25, 4.0f, 0.25f, 1.5f, babyCopperShrubColor, copperShrubColor, deadCopperShrubColor, 0.2f, 3.0f, 10, 10, "Copper shrub");

		RGBA babyIronShrubColor = RGBA(96, 192, 225), ironShrubColor = RGBA(67, 90, 99), deadIronShrubColor = RGBA(45, 47, 48);
		Shrub* ironShrub = new Shrub(Resources::iron->Clone(), ItemInstance(ITEMTYPE::IRON_SHRUB_SEED), 120, 180, 10, 0.5f, 0.25f, 1.5f, babyIronShrubColor, ironShrubColor, deadIronShrubColor, 0.2f, 3.0f, 10, 10, "Iron shrub");

		RGBA babyRubyShrubColor = RGBA(207, 120, 156), rubyShrubColor = RGBA(135, 16, 66), deadRubyShrubColor = RGBA(120, 65, 88);
		Shrub* rubyShrub = new Shrub(Resources::ruby->Clone(), ItemInstance(ITEMTYPE::RUBY_SHRUB_SEED), 5, 15, 50, 4.0f, 0.25f, 1.5f, babyRubyShrubColor, rubyShrubColor, deadRubyShrubColor, 0.2f, 3.0f, 10, 10, "Ruby shrub");

		RGBA babyEmeraldShrubColor = RGBA(145, 255, 204), emeraldShrubColor = RGBA(65, 166, 119), deadEmeraldShrubColor = RGBA(61, 97, 80);
		Shrub* emeraldShrub = new Shrub(Resources::emerald->Clone(), ItemInstance(ITEMTYPE::EMERALD_SHRUB_SEED), 5, 15, 50, 4.0f, 0.25f, 1.5f, babyEmeraldShrubColor, emeraldShrubColor, deadEmeraldShrubColor, 0.2f, 3.0f, 10, 10, "Emerald shrub");

		RGBA babyRockShrubColor = RGBA(212, 212, 212), rockShrubColor = RGBA(201, 196, 165), deadRockShrubColor = RGBA(130, 130, 130);
		Shrub* rockShrub = new Shrub(Resources::rock->Clone(), ItemInstance(ITEMTYPE::ROCK_SHRUB_SEED), 5, 14, 25, 3.0f, 0.25f, 1.5f, babyRockShrubColor, rockShrubColor, deadRockShrubColor, 0.2f, 3.0f, 10, 10, "Rock shrub");
		
		RGBA babyShadeShrubColor = RGBA(50, 50), shadeShrubColor = RGBA(25, 25), deadShadeShrubColor = RGBA();
		Shrub* shadeShrub = new Shrub(Resources::shade->Clone(), ItemInstance(ITEMTYPE::SHADE_SHRUB_SEED), 10, 30, 25, 1.0f, 0.25f, 1.5f, babyShadeShrubColor, shadeShrubColor, deadShadeShrubColor, 0.2f, 3.0f, 10, 10, "Shade shrub");
	
		RGBA babyBowlerShrubColor = RGBA(111, 101, 143), bowlerShrubColor = RGBA(21, 0, 89), deadBowlerShrub = RGBA(12, 4, 36);
		Shrub* bowlerShrub = new Shrub(Resources::bowler->Clone(), ItemInstance(ITEMTYPE::BOWLER_SHRUB_SEED), 1, 50, 50, 16.0f, 0.5f, 2.5f, babyBowlerShrubColor, bowlerShrubColor, deadBowlerShrub, 1, 5, 50, 50, "Bowler shrub");
	
		RGBA babyVacuumiumShrubColor = RGBA(242, 239, 148), vacuumiumShrubColor = RGBA(204, 202, 153), deadVacuumiumShrubColor = RGBA(158, 156, 85);
		Shrub* vacuumiumShrub = new Shrub(Resources::vacuumium->Clone(), ItemInstance(ITEMTYPE::VACUUMIUM_SHRUB_SEED), 10, 60, 5, 0.5f, 2.f, 0.25f, babyVacuumiumShrubColor, vacuumiumShrubColor, deadVacuumiumShrubColor, 0.2f, 3.0f, 60, 60, "Vacuumium shrub");
	
		RGBA babySilverShrubColor = RGBA(209, 157, 157), silverShrubColor = RGBA(171, 171, 171), deadSilverShrubColor = RGBA(87, 99, 74);
		Shrub* silverShrub = new Shrub(Resources::silver->Clone(), ItemInstance(ITEMTYPE::SILVER_SHRUB_SEED), 4, 54, 10, 10.f, 0.125f, 1.f, babySilverShrubColor, silverShrubColor, deadSilverShrubColor, 0.5f, 1.f, 30, 30, "Silver tree");
		
		RGBA babyQuartzShrubColor = RGBA(202, 188, 224), quartzShrubColor = RGBA(161, 153, 173), deadQuartzColor = RGBA(127, 70, 212);
		Shrub* quartzShrub = new Shrub(Resources::quartz->Clone(), ItemInstance(ITEMTYPE::QUARTZ_SHRUB_SEED), 3, 12, 25, 3.f, 0.25f, 0.5f, babyQuartzShrubColor, quartzShrubColor, deadQuartzColor, 0.25f, 0.5f, 10, 10, "Quartz tree");
	}


	namespace Vines
	{
		RGBA babyCheeseVineColor = RGBA(255, 210, 112), cheeseVineColor = RGBA(200, 160, 75), deadCheeseVineColor = RGBA(140, 110, 50);
		Vine* cheeseVine = new Vine(Resources::cheese->Clone(), ItemInstance(ITEMTYPE::CHEESE_VINE_SEED), 0.0f, 25, 1, 25, 25, 2.0f, 0.25f, 0.5f, babyCheeseVineColor, cheeseVineColor, deadCheeseVineColor, 1, 10, 10, "Cheese vine");

		RGBA babyLeadVineColor = RGBA(198, 111, 227), leadVineColor = RGBA(153, 29, 194), deadLeadVineColor = RGBA(15, 50, 61);
		Vine* leadVine = new Vine(Resources::lead->Clone(), ItemInstance(ITEMTYPE::LEAD_VINE_SEED), 0.8f, 25, 1, 25, 2, 3.0f, 0.25f, 0.5f, babyLeadVineColor, leadVineColor, deadLeadVineColor, 2, 10, 10, "Lead vine");
		
		RGBA babyTopazVineColor = RGBA(255, 218, 84), topazVineColor = RGBA(181, 142, 0), deadTopazVineColor = RGBA(107, 84, 0);
		Vine* topazVine = new Vine(Resources::topaz->Clone(2), ItemInstance(ITEMTYPE::TOPAZ_VINE_SEED), 0.8f, 25, 1, 50, 5, 2.0f, 0.5f, 1.5f, babyTopazVineColor, topazVineColor, deadTopazVineColor, 5, 60, 60, "Topaz vine");
		
		RGBA babySapphireVineColor = RGBA(125, 91, 212), sapphireVineColor = RGBA(132, 89, 255), deadSapphireVineColor = RGBA(75, 69, 92);
		Vine* sapphireVine = new Vine(Resources::sapphire->Clone(5), ItemInstance(ITEMTYPE::SAPPHIRE_VINE_SEED), 0.8f, 25, 3, 25, 15, 0.125f, 0.25f, 0.5f, babySapphireVineColor, sapphireVineColor, deadSapphireVineColor, 1, 40, 40, "Sapphire vine");
		
		RGBA babyQuartzVineColor = RGBA(202, 188, 224), quartzVineColor = RGBA(161, 153, 173), deadQuartzColor = RGBA(127, 70, 212);
		Vine* quartzVine = new Vine(Resources::quartz->Clone(3), ItemInstance(ITEMTYPE::QUARTZ_VINE_SEED), 1.6f, 50, 3, 10, 25, 0.25f, 0.25f, 0.5f, babyQuartzVineColor, quartzVineColor, deadQuartzColor, 1, 10, 10, "Quarts vine");
	}

	// Keep a list of all of the plants. Shrub is the base of all plants so it's what we'll use for the pointer.
	vector<Shrub*> plants{ Shrubs::copperShrub, Shrubs::ironShrub, Shrubs::rubyShrub, Shrubs::emeraldShrub, Shrubs::rockShrub, Shrubs::shadeShrub,
		Shrubs::bowlerShrub, Shrubs::vacuumiumShrub, Shrubs::silverShrub, Shrubs::quartzShrub,
		Vines::cheeseVine, Vines::topazVine, Vines::sapphireVine, Vines::leadVine, Vines::quartzVine };
}

namespace Resources::Seeds
{
	// Shrubs
	PlacedOnLanding* copperShrubSeed = new PlacedOnLanding(ITEMTYPE::COPPER_SHRUB_SEED, Plants::Shrubs::copperShrub, "Copper shrub seed", "Seed", 4, Plants::Shrubs::copperShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::copperShrub->babyRadius);
	PlacedOnLanding* ironShrubSeed = new PlacedOnLanding(ITEMTYPE::IRON_SHRUB_SEED, Plants::Shrubs::ironShrub, "Iron shrub seed", "Seed", 4, Plants::Shrubs::ironShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::ironShrub->babyRadius);
	PlacedOnLanding* rockShrubSeed = new PlacedOnLanding(ITEMTYPE::ROCK_SHRUB_SEED, Plants::Shrubs::rockShrub, "Rock shrub seed", "Seed", 4, Plants::Shrubs::rockShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::rockShrub->babyRadius);
	CorruptOnKill* rubyShrubSeed = new CorruptOnKill(ITEMTYPE::RUBY_SHRUB_SEED, Plants::Shrubs::rubyShrub, "Ruby shrub seed", "Corruption Seed", 2, Plants::Shrubs::rubyShrubColor, 10, 15, false, 0.25f, 12.f, Plants::Shrubs::rubyShrub->babyRadius);
	CorruptOnKill* emeraldShrubSeed = new CorruptOnKill(ITEMTYPE::EMERALD_SHRUB_SEED, Plants::Shrubs::emeraldShrub, "Emerald shrub seed", "Corruption Seed", 2, Plants::Shrubs::emeraldShrubColor, 10, 15, false, 0.25f, 12.f, Plants::Shrubs::emeraldShrub->babyRadius);
	PlacedOnLanding* shadeShrubSeed = new PlacedOnLanding(ITEMTYPE::SHADE_SHRUB_SEED, Plants::Shrubs::shadeShrub, "Shade shrub seed", "Seed", 4, Plants::Shrubs::shadeShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::shadeShrub->babyRadius);
	PlacedOnLanding* bowlerShrubSeed = new PlacedOnLanding(ITEMTYPE::BOWLER_SHRUB_SEED, Plants::Shrubs::bowlerShrub, "Bowler shrub seed", "Seed", 4, Plants::Shrubs::bowlerShrubColor, 0, 15, false, 0.5f, 12.f, Plants::Shrubs::bowlerShrub->babyRadius);
	PlacedOnLanding* vacuumiumShrubSeed = new PlacedOnLanding(ITEMTYPE::VACUUMIUM_SHRUB_SEED, Plants::Shrubs::vacuumiumShrub, "Vacuumium shrub seed", "Seed", 4, Plants::Shrubs::vacuumiumShrubColor, 0, 15, false, 0.5f, 12.f, Plants::Shrubs::vacuumiumShrub->babyRadius);
	PlacedOnLanding* silverShrubSeed = new PlacedOnLanding(ITEMTYPE::SILVER_SHRUB_SEED, Plants::Shrubs::silverShrub, "Silver shrub seed", "Seed", 4, Plants::Shrubs::silverShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::silverShrub->babyRadius);
	PlacedOnLanding* quartzShrubSeed = new PlacedOnLanding(ITEMTYPE::QUARTZ_SHRUB_SEED, Plants::Shrubs::quartzShrub, "Quartz shrub seed", "Seed", 4, Plants::Shrubs::quartzShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::quartzShrub->babyRadius);
	// Vines
	PlacedOnLanding* cheeseVineSeed = new PlacedOnLanding(ITEMTYPE::CHEESE_VINE_SEED, Plants::Vines::cheeseVine, "Cheese vine seed", "Seed", 4, Plants::Vines::cheeseVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::cheeseVine->babyRadius);
	PlacedOnLanding* topazVineSeed = new PlacedOnLanding(ITEMTYPE::TOPAZ_VINE_SEED, Plants::Vines::topazVine, "Topaz vine seed", "Seed", 4, Plants::Vines::topazVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::topazVine->babyRadius);
	CorruptOnKill* sapphireVineSeed = new CorruptOnKill(ITEMTYPE::SAPPHIRE_VINE_SEED, Plants::Vines::sapphireVine, "Sapphire vine seed", "Corruption Seed", 2, Plants::Vines::sapphireVineColor, 10, 15.f, false, 0.25f, 12.f, Plants::Vines::sapphireVine->babyRadius);
	PlacedOnLanding* leadVineSeed = new PlacedOnLanding(ITEMTYPE::LEAD_VINE_SEED, Plants::Vines::leadVine, "Lead vine seed", "Seed", 4, Plants::Vines::leadVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::leadVine->babyRadius);
	PlacedOnLanding* quartzVineSeed = new PlacedOnLanding(ITEMTYPE::QUARTZ_VINE_SEED, Plants::Vines::quartzVine, "Quartz vine seed", "Seed", 4, Plants::Vines::quartzVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::quartzVine->babyRadius);

	// Keep a list of all of the seeds.
	vector<Item*> plantSeeds{ copperShrubSeed, ironShrubSeed, rubyShrubSeed, emeraldShrubSeed, rockShrubSeed, shadeShrubSeed, bowlerShrubSeed,
		vacuumiumShrubSeed, silverShrubSeed, quartzShrubSeed, cheeseVineSeed, topazVineSeed, sapphireVineSeed, leadVineSeed, quartzVineSeed };
	
}

namespace Collectibles::Seeds
{
	// Vines
	Collectible* copperShrubSeed = new Collectible(Resources::Seeds::copperShrubSeed->Clone());
	Collectible* ironShrubSeed = new Collectible(Resources::Seeds::ironShrubSeed->Clone());
	Collectible* rubyShrubSeed = new Collectible(Resources::Seeds::rubyShrubSeed->Clone());
	Collectible* emeraldShrubSeed = new Collectible(Resources::Seeds::emeraldShrubSeed->Clone());
	Collectible* rockShrubSeed = new Collectible(Resources::Seeds::rockShrubSeed->Clone());
	Collectible* shadeShrubSeed = new Collectible(Resources::Seeds::shadeShrubSeed->Clone());
	Collectible* bowlerShrubSeed = new Collectible(Resources::Seeds::bowlerShrubSeed->Clone());
	Collectible* vacuumiumShrubSeed = new Collectible(Resources::Seeds::vacuumiumShrubSeed->Clone());
	Collectible* silverShrubSeed = new Collectible(Resources::Seeds::silverShrubSeed->Clone());
	Collectible* quartzShrubSeed = new Collectible(Resources::Seeds::quartzShrubSeed->Clone());
	// Vines
	Collectible* cheeseVineSeed = new Collectible(Resources::Seeds::cheeseVineSeed->Clone());
	Collectible* topazVineSeed = new Collectible(Resources::Seeds::topazVineSeed->Clone());
	Collectible* sapphireVineSeed = new Collectible(Resources::Seeds::sapphireVineSeed->Clone());
	Collectible* leadVineSeed = new Collectible(Resources::Seeds::leadVineSeed->Clone());
	Collectible* quartzVineSeed = new Collectible(Resources::Seeds::quartzVineSeed->Clone());

	// Keep a list of all of the seeds.
	vector<Collectible*> plantSeeds{ copperShrubSeed, ironShrubSeed, rubyShrubSeed, emeraldShrubSeed, rockShrubSeed, shadeShrubSeed, bowlerShrubSeed,
		vacuumiumShrubSeed, silverShrubSeed, quartzShrubSeed, cheeseVineSeed, topazVineSeed, sapphireVineSeed, leadVineSeed, quartzVineSeed };
}

#pragma endregion