#include "BuildingBlocks.h"

#pragma region Plantlife

float difficultyGrowthModifier[] = { 2.0f, 1.0f, 0.5f };

FunctionalBlockData shrubData = FunctionalBlockData(TUPDATE::SHRUB, UPDATE::FUNCTIONALBLOCK2, VUPDATE::FRICTION, DUPDATE::SHRUB, EDUPDATE::ENTITY, UIUPDATE::SHRUB);
class Shrub : public FunctionalBlock2
{
public:
	ItemInstance collectible, seed;
	RGBA adultColor, deadColor;
	int cyclesToGrow, deadStage, currentLifespan, chanceForSeed;
	float babyRadius, maxRadius, babyMass, maxMass;
	Vec3 nextPlacementPos = vZero;
	bool nextSpawnSeed = true;

	Shrub(EntityData* data, ItemInstance collectible, ItemInstance seed, int cyclesToGrow, int deadStage, int chanceForSeed,
		float timePer, float radius = 0.5f, float maxRadius = 0.5f, RGBA color = RGBA(), RGBA adultColor = RGBA(), RGBA deadColor = RGBA(),
		float mass = 1, float maxMass = 2, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		collectible(collectible), seed(seed), cyclesToGrow(cyclesToGrow), deadStage(deadStage),
		currentLifespan(0), chanceForSeed(chanceForSeed), adultColor(adultColor), deadColor(deadColor),
		babyRadius(radius), maxRadius(maxRadius), babyMass(mass), maxMass(maxMass),
		FunctionalBlock2(data, timePer, vZero, radius, color, mass, bounciness, maxHealth, health, name) { }

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

/*class Tree : public FunctionalBlock2
{
public:
	Tree* trunk = nullptr, *last = nullptr;
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
		float mass, float bounciness, int maxHealth, int health, string name) :
		collectible(collectible), seed(seed),
		height(height), currentHeight(0), leafLength(leafLength), chanceForSeed(chanceForSeed), branchCount(branchCount),
		leafRadius(leafRadius), leafColor(leafColor),
		FunctionalBlock2(timePer, vZero, radius, color, mass, bounciness, maxHealth, health, name)
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
		/*else *nextPlacementPos = up * (radius + (currentHeight + 1 > height ? leafRadius : radius));
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
};*/

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
		/*Tree* tree = static_cast<Tree*>(entity);
		if (tree->radius != tree->baseClass->radius)
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
		}
		tree->DUpdate(DUPDATE::ENTITY);*/
	}
}


FunctionalBlockData vineData = FunctionalBlockData(TUPDATE::VINE, UPDATE::VINE, VUPDATE::VINE, DUPDATE::SHRUB, EDUPDATE::ENTITY, UIUPDATE::SHRUB, ONDEATH::ENTITY);
class Vine : public Shrub
{
public:
	Vine* base = nullptr, * back = nullptr, * front = nullptr;
	float angleWobble;
	int maxGenerations, generation;
	sByte nextSide = 0;

	Vine(EntityData* data, ItemInstance collectible, ItemInstance seed, float angleWobble, int cyclesToGrow, int deadStage, int maxGenerations, int chanceForSeed,
		float timePer, float radius = 0.5f, float maxRadius = 0.5f, RGBA color = RGBA(), RGBA adultColor = RGBA(),
		RGBA deadColor = RGBA(),
		float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Shrub(data, collectible, seed, cyclesToGrow, deadStage, chanceForSeed, timePer, radius, maxRadius, color, adultColor, deadColor,
			mass, mass, bounciness, maxHealth, health, name),
		maxGenerations(maxGenerations), generation(0), angleWobble(angleWobble) { }

	void Start() override
	{
		Shrub::Start();
		dir = glm::rotateZ(dir, (RandFloat() * 2 - 1) * angleWobble);
		nextPlacementPos = dir;
		radius = babyRadius;
		nextSpawnSeed = true;
	}

	Vine(Vine* baseClass, Vec3 dir, Vec3 pos) :
		Vine(*baseClass)
	{
		this->pos = pos;
		dir.z = 0;
		this->dir = Normalized(dir);
		if (this->dir == vZero) this->dir = RandCircPoint();
		this->baseClass = baseClass;
		base = this;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Vine>(this, dir, pos);
	}

	void UnAttach(Entity* entity) override
	{
		if (entity == base)
		{
			health = 0; // Set to 0 but may be further decreased before frame ends.
			DelayedDestroySelf();
		}
	}

	int ApplyHit(int damage, Entity* damageDealer) override
	{
		if (health <= 0) return 1;
		if (back != nullptr)
			return base->ApplyHit(damage, damageDealer);

		int result = Shrub::ApplyHitHarmless(damage, damageDealer);
		if (result == 1)
		{
			health = 0;
			OnDeath(damageDealer);
			DelayedDestroySelf();
		}
		return result;
	}
};

namespace Updates
{
	void VineU(Entity* entity)
	{
		Vine* vine = static_cast<Vine*>(entity);
		if (vine->front == nullptr)
		{
			if (vine->back != nullptr)
			{
				vine->dir = Normalized(vine->pos - vine->back->pos);
				vine->nextPlacementPos = vine->dir;
			}
		}
		else
			vine->nextPlacementPos = glm::rotateZ(vine->dir, vine->nextSide * PI_F * 0.5f);
		vine->Update(UPDATE::FUNCTIONALBLOCK2);
	}
}

namespace VUpdates
{
	void VineVU(Entity* entity)
	{
		Vine* vine = static_cast<Vine*>(entity);

		if (vine->back == nullptr)
		{
			for (int i = 0; i < 2; i++)
			{
				Vine* currentFarthest = vine->front;
				while (currentFarthest)
				{
					Vine* back = currentFarthest->back;
					float dist = glm::distance(back->pos, currentFarthest->pos);
					Vec3 multiplier = (back->pos - currentFarthest->pos) * (1.01f * (dist - currentFarthest->radius - back->radius) / (dist * (currentFarthest->mass + back->mass)));
					currentFarthest->SetPos(currentFarthest->pos + multiplier * back->mass);
					back->SetPos(back->pos - multiplier * currentFarthest->mass);
					currentFarthest = currentFarthest->front;
				}
			}
		}
		vine->VUpdate(VUPDATE::FRICTION);
	}
}

namespace OnDeaths
{
	void VineOD(Entity* entity, Entity* damageDealer)
	{
		/*Vine* vine = static_cast<Vine*>(entity);
		if (rand() % 100 < vine->chanceForSeed)
			game->entities->push_back(make_unique<Collectible>(vine->seed, vine->pos));
		else
			game->entities->push_back(make_unique<Collectible>(vine->collectible, vine->pos));*/
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
		/*Tree* tree = static_cast<Tree*>(entity);
		if (tree->currentLifespan >= tree->cyclesToGrow && tree->currentLifespan < tree->deadStage)
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
		if (vine->generation >= vine->maxGenerations)
			vine->currentLifespan = vine->deadStage;

		if (vine->currentLifespan >= vine->cyclesToGrow && vine->currentLifespan < vine->deadStage)
		{
			if (vine->currentLifespan <= vine->cyclesToGrow)
			{
				unique_ptr<Entity> newEntity = vine->baseClass->Clone(placementPos, vine->dir, vine);
				Vine* newVine = static_cast<Vine*>(newEntity.get());
				newVine->generation = vine->generation + 1;
				newVine->base = vine->base;
				newVine->back = vine;
				vine->front = newVine;
				vine->base->observers.push_back(newVine);
				vine->base->maxHealth += newVine->health;
				vine->base->health += newVine->health;
				game->entities->push_back(std::move(newEntity));
			}
			else if (vine->nextSpawnSeed)
				game->entities->push_back(make_unique<Collectible>(vine->seed.Clone(), vine->pos + (vine->radius + vine->seed->radius) * vine->nextPlacementPos));
			else
				game->entities->push_back(make_unique<Collectible>(vine->collectible.Clone(), vine->pos + (vine->radius + vine->collectible->radius) * vine->nextPlacementPos));
			
			vine->nextSide = rand() % 2 * 2 - 1;
			vine->nextSpawnSeed = rand() % 100 < vine->chanceForSeed;
		}
		vine->currentLifespan++;
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
		/*Tree* tree = static_cast<Tree*>(entity);

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
	*/}

	void VineUIU(Entity* entity)
	{
		/*(Vine * vine = static_cast<Vine*>(entity);
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
		}*/
	}
}


#pragma region Plants
// Having an enum of all seeds will come in handy:
enum class SEEDINDICES
{
	COPPER, IRON, RUBY, EMERALD, ROCK, SHADE, BOWLER, VACUUMIUM, SILVER, QUARTZ_S, COAL, CHEESE, TOPAZ, SAPPHIRE, LEAD, QUARTZ_V
};


namespace Plants
{
	namespace Shrubs
	{
		RGBA babyCopperShrubColor = RGBA(207, 137, 81), copperShrubColor = RGBA(163, 78, 8), deadCopperShrubColor = RGBA(94, 52, 17);
		Shrub copperShrub = Shrub(&shrubData, Resources::copper.Clone(), ItemInstance(ITEMTYPE::COPPER_SHRUB_SEED), 5, 25, 25, 4.0f, 0.25f, 1.5f, babyCopperShrubColor, copperShrubColor, deadCopperShrubColor, 0.2f, 3.0f, 0, 10, 10, "Copper shrub");

		RGBA babyIronShrubColor = RGBA(96, 192, 225), ironShrubColor = RGBA(67, 90, 99), deadIronShrubColor = RGBA(45, 47, 48);
		Shrub ironShrub = Shrub(&shrubData, Resources::iron.Clone(), ItemInstance(ITEMTYPE::IRON_SHRUB_SEED), 120, 180, 10, 0.5f, 0.25f, 1.5f, babyIronShrubColor, ironShrubColor, deadIronShrubColor, 0.2f, 3.0f, 0, 10, 10, "Iron shrub");

		RGBA babyRubyShrubColor = RGBA(207, 120, 156), rubyShrubColor = RGBA(135, 16, 66), deadRubyShrubColor = RGBA(120, 65, 88);
		Shrub rubyShrub = Shrub(&shrubData, Resources::ruby.Clone(), ItemInstance(ITEMTYPE::RUBY_SHRUB_SEED), 5, 15, 50, 4.0f, 0.25f, 1.5f, babyRubyShrubColor, rubyShrubColor, deadRubyShrubColor, 0.2f, 3.0f, 0, 10, 10, "Ruby shrub");

		RGBA babyEmeraldShrubColor = RGBA(145, 255, 204), emeraldShrubColor = RGBA(65, 166, 119), deadEmeraldShrubColor = RGBA(61, 97, 80);
		Shrub emeraldShrub = Shrub(&shrubData, Resources::emerald.Clone(), ItemInstance(ITEMTYPE::EMERALD_SHRUB_SEED), 5, 15, 50, 4.0f, 0.25f, 1.5f, babyEmeraldShrubColor, emeraldShrubColor, deadEmeraldShrubColor, 0.2f, 3.0f, 0, 10, 10, "Emerald shrub");

		RGBA babyRockShrubColor = RGBA(212, 212, 212), rockShrubColor = RGBA(201, 196, 165), deadRockShrubColor = RGBA(130, 130, 130);
		Shrub rockShrub = Shrub(&shrubData, Resources::rock.Clone(), ItemInstance(ITEMTYPE::ROCK_SHRUB_SEED), 5, 14, 25, 3.0f, 0.25f, 1.5f, babyRockShrubColor, rockShrubColor, deadRockShrubColor, 0.2f, 3.0f, 0, 10, 10, "Rock shrub");
		
		RGBA babyShadeShrubColor = RGBA(50, 50), shadeShrubColor = RGBA(25, 25), deadShadeShrubColor = RGBA();
		Shrub shadeShrub = Shrub(&shrubData, Resources::shade.Clone(), ItemInstance(ITEMTYPE::SHADE_SHRUB_SEED), 10, 30, 25, 1.0f, 0.25f, 1.5f, babyShadeShrubColor, shadeShrubColor, deadShadeShrubColor, 0.2f, 3.0f, 0, 10, 10, "Shade shrub");
	
		RGBA babyBowlerShrubColor = RGBA(111, 101, 143), bowlerShrubColor = RGBA(21, 0, 89), deadBowlerShrub = RGBA(12, 4, 36);
		Shrub bowlerShrub = Shrub(&shrubData, Resources::bowler.Clone(), ItemInstance(ITEMTYPE::BOWLER_SHRUB_SEED), 1, 50, 50, 16.0f, 0.5f, 2.5f, babyBowlerShrubColor, bowlerShrubColor, deadBowlerShrub, 1, 5, 0, 50, 50, "Bowler shrub");
	
		RGBA babyVacuumiumShrubColor = RGBA(242, 239, 148), vacuumiumShrubColor = RGBA(204, 202, 153), deadVacuumiumShrubColor = RGBA(158, 156, 85);
		Shrub vacuumiumShrub = Shrub(&shrubData, Resources::vacuumium.Clone(), ItemInstance(ITEMTYPE::VACUUMIUM_SHRUB_SEED), 10, 60, 5, 0.5f, 2.f, 0.25f, babyVacuumiumShrubColor, vacuumiumShrubColor, deadVacuumiumShrubColor, 0.2f, 3.0f, 0, 60, 60, "Vacuumium shrub");
	
		RGBA babySilverShrubColor = RGBA(209, 157, 157), silverShrubColor = RGBA(171, 171, 171), deadSilverShrubColor = RGBA(87, 99, 74);
		Shrub silverShrub = Shrub(&shrubData, Resources::silver.Clone(), ItemInstance(ITEMTYPE::SILVER_SHRUB_SEED), 4, 54, 10, 10.f, 0.125f, 1.f, babySilverShrubColor, silverShrubColor, deadSilverShrubColor, 0.5f, 1.f, 0, 30, 30, "Silver shrub");
		
		RGBA babyQuartzShrubColor = RGBA(202, 188, 224), quartzShrubColor = RGBA(161, 153, 173), deadQuartzShrubColor = RGBA(127, 70, 212);
		Shrub quartzShrub = Shrub(&shrubData, Resources::quartz.Clone(), ItemInstance(ITEMTYPE::QUARTZ_SHRUB_SEED), 3, 12, 25, 3.f, 0.25f, 0.5f, babyQuartzShrubColor, quartzShrubColor, deadQuartzShrubColor, 0.25f, 0.5f, 0, 10, 10, "Quartz shrub");
	
		RGBA babyCoalShrubColor = RGBA(99, 66, 20), coalShrubColor = RGBA(59, 44, 23), deadCoalShrubColor = RGBA(74, 65, 52);
		Shrub coalShrub = Shrub(&shrubData, dItem->Clone(), ItemInstance(ITEMTYPE::COAL), 10, 15, 100, 3.f, 0.25f, 1.5f, babyCoalShrubColor, coalShrubColor, deadCoalShrubColor, 1, 5, 0, 50, 50, "Coal shrub");
	}


	namespace Vines
	{
		RGBA babyCheeseVineColor = RGBA(255, 210, 112), cheeseVineColor = RGBA(200, 160, 75), deadCheeseVineColor = RGBA(140, 110, 50);
		Vine cheeseVine = Vine(&vineData, Resources::cheese.Clone(), ItemInstance(ITEMTYPE::CHEESE_VINE_SEED), 0.8f, 2, 5, 25, 25, 2.0f, 0.25f, 0.5f, babyCheeseVineColor, cheeseVineColor, deadCheeseVineColor, 1, 0, 10, 10, "Cheese vine");

		RGBA babyLeadVineColor = RGBA(198, 111, 227), leadVineColor = RGBA(153, 29, 194), deadLeadVineColor = RGBA(15, 50, 61);
		Vine leadVine = Vine(&vineData, Resources::lead.Clone(), ItemInstance(ITEMTYPE::LEAD_VINE_SEED), 0.8f, 1, 4, 25, 25, 3.0f, 0.25f, 0.5f, babyLeadVineColor, leadVineColor, deadLeadVineColor, 2, 0, 10, 10, "Lead vine");
		
		RGBA babyTopazVineColor = RGBA(255, 218, 84), topazVineColor = RGBA(181, 142, 0), deadTopazVineColor = RGBA(107, 84, 0);
		Vine topazVine = Vine(&vineData, Resources::topaz.Clone(2), ItemInstance(ITEMTYPE::TOPAZ_VINE_SEED), 0.8f, 1, 6, 50, 10, 2.0f, 0.25f, 1.5f, babyTopazVineColor, topazVineColor, deadTopazVineColor, 5, 0, 60, 60, "Topaz vine");
		
		RGBA babySapphireVineColor = RGBA(125, 91, 212), sapphireVineColor = RGBA(132, 89, 255), deadSapphireVineColor = RGBA(75, 69, 92);
		Vine sapphireVine = Vine(&vineData, Resources::sapphire.Clone(5), ItemInstance(ITEMTYPE::SAPPHIRE_VINE_SEED), 0.8f, 3, 6, 25, 15, 0.125f, 0.25f, 0.5f, babySapphireVineColor, sapphireVineColor, deadSapphireVineColor, 1, 0, 40, 40, "Sapphire vine");
		
		RGBA babyQuartzVineColor = RGBA(202, 188, 224), quartzVineColor = RGBA(161, 153, 173), deadQuartzColor = RGBA(127, 70, 212);
		Vine quartzVine = Vine(&vineData, Resources::quartz.Clone(3), ItemInstance(ITEMTYPE::QUARTZ_VINE_SEED), 1.6f, 3, 5, 10, 25, 0.25f, 0.25f, 0.5f, babyQuartzVineColor, quartzVineColor, deadQuartzColor, 1, 0, 10, 10, "Quarts vine");
	}

	// Keep a list of all of the plants. Shrub is the base of all plants so it's what we'll use for the pointer.
	vector<Shrub*> plants{ &Shrubs::copperShrub, &Shrubs::ironShrub, &Shrubs::rubyShrub, &Shrubs::emeraldShrub, &Shrubs::rockShrub,
		&Shrubs::shadeShrub, &Shrubs::bowlerShrub, &Shrubs::vacuumiumShrub, &Shrubs::silverShrub, &Shrubs::quartzShrub, &Shrubs::coalShrub,
		&Vines::cheeseVine, &Vines::topazVine, &Vines::sapphireVine, &Vines::leadVine, &Vines::quartzVine };
}

namespace Resources::Seeds
{
	// Shrubs
	PlacedOnLanding copperShrubSeed = PlacedOnLanding(ITEMTYPE::COPPER_SHRUB_SEED, &Plants::Shrubs::copperShrub, "Copper shrub seed", "Seed", 4, Plants::Shrubs::copperShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::copperShrub.babyRadius, false, true, true);
	PlacedOnLanding ironShrubSeed = PlacedOnLanding(ITEMTYPE::IRON_SHRUB_SEED, &Plants::Shrubs::ironShrub, "Iron shrub seed", "Seed", 4, Plants::Shrubs::ironShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::ironShrub.babyRadius, false, true, true);
	PlacedOnLanding rockShrubSeed = PlacedOnLanding(ITEMTYPE::ROCK_SHRUB_SEED, &Plants::Shrubs::rockShrub, "Rock shrub seed", "Seed", 4, Plants::Shrubs::rockShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::rockShrub.babyRadius, false, true, true);
	CorruptOnKill rubyShrubSeed = CorruptOnKill(ITEMTYPE::RUBY_SHRUB_SEED, &Plants::Shrubs::rubyShrub, "Ruby shrub seed", "Corruption Seed", 2, Plants::Shrubs::rubyShrubColor, 10, 15, false, 0.25f, 12.f, Plants::Shrubs::rubyShrub.babyRadius);
	CorruptOnKill emeraldShrubSeed = CorruptOnKill(ITEMTYPE::EMERALD_SHRUB_SEED, &Plants::Shrubs::emeraldShrub, "Emerald shrub seed", "Corruption Seed", 2, Plants::Shrubs::emeraldShrubColor, 10, 15, false, 0.25f, 12.f, Plants::Shrubs::emeraldShrub.babyRadius);
	PlacedOnLanding shadeShrubSeed = PlacedOnLanding(ITEMTYPE::SHADE_SHRUB_SEED, &Plants::Shrubs::shadeShrub, "Shade shrub seed", "Seed", 4, Plants::Shrubs::shadeShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::shadeShrub.babyRadius, false, true, true);
	PlacedOnLanding bowlerShrubSeed = PlacedOnLanding(ITEMTYPE::BOWLER_SHRUB_SEED, &Plants::Shrubs::bowlerShrub, "Bowler shrub seed", "Seed", 4, Plants::Shrubs::bowlerShrubColor, 0, 15, false, 0.5f, 12.f, Plants::Shrubs::bowlerShrub.babyRadius, false, true, true);
	PlacedOnLanding vacuumiumShrubSeed = PlacedOnLanding(ITEMTYPE::VACUUMIUM_SHRUB_SEED, &Plants::Shrubs::vacuumiumShrub, "Vacuumium shrub seed", "Seed", 4, Plants::Shrubs::vacuumiumShrubColor, 0, 15, false, 0.5f, 12.f, Plants::Shrubs::vacuumiumShrub.babyRadius, false, true, true);
	PlacedOnLanding silverShrubSeed = PlacedOnLanding(ITEMTYPE::SILVER_SHRUB_SEED, &Plants::Shrubs::silverShrub, "Silver shrub seed", "Seed", 4, Plants::Shrubs::silverShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::silverShrub.babyRadius, false, true, true);
	PlacedOnLanding quartzShrubSeed = PlacedOnLanding(ITEMTYPE::QUARTZ_SHRUB_SEED, &Plants::Shrubs::quartzShrub, "Quartz shrub seed", "Seed", 4, Plants::Shrubs::quartzShrubColor, 0, 15, false, 0.25f, 12.f, Plants::Shrubs::quartzShrub.babyRadius, false, true, true);
	PlacedOnLandingBoom coal = PlacedOnLandingBoom(ITEMTYPE::COAL, 6.f, 30, &Plants::Shrubs::coalShrub, "Coal shrub seed", "Seed Ammo", 4, Plants::Shrubs::coalShrubColor, 20, 10.f, false, 0.25f, 24.f, Plants::Shrubs::coalShrub.babyRadius, false, true, true);

	// Vines
	PlacedOnLanding cheeseVineSeed = PlacedOnLanding(ITEMTYPE::CHEESE_VINE_SEED, &Plants::Vines::cheeseVine, "Cheese vine seed", "Seed", 4, Plants::Vines::cheeseVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::cheeseVine.babyRadius, false, true, true);
	PlacedOnLanding topazVineSeed = PlacedOnLanding(ITEMTYPE::TOPAZ_VINE_SEED, &Plants::Vines::topazVine, "Topaz vine seed", "Seed", 4, Plants::Vines::topazVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::topazVine.babyRadius, false, true, true);
	CorruptOnKill sapphireVineSeed = CorruptOnKill(ITEMTYPE::SAPPHIRE_VINE_SEED, &Plants::Vines::sapphireVine, "Sapphire vine seed", "Corruption Seed", 2, Plants::Vines::sapphireVineColor, 10, 15.f, false, 0.25f, 12.f, Plants::Vines::sapphireVine.babyRadius);
	PlacedOnLanding leadVineSeed = PlacedOnLanding(ITEMTYPE::LEAD_VINE_SEED, &Plants::Vines::leadVine, "Lead vine seed", "Seed", 4, Plants::Vines::leadVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::leadVine.babyRadius, false, true, true);
	PlacedOnLanding quartzVineSeed = PlacedOnLanding(ITEMTYPE::QUARTZ_VINE_SEED, &Plants::Vines::quartzVine, "Quartz vine seed", "Seed", 4, Plants::Vines::quartzVineColor, 0, 15.f, false, 0.25f, 12.f, Plants::Vines::quartzVine.babyRadius, false, true, true);

	// Keep a list of all of the seeds.
	vector<Item*> plantSeeds{ &copperShrubSeed, &ironShrubSeed, &rubyShrubSeed, &emeraldShrubSeed, &rockShrubSeed, &shadeShrubSeed,
		&bowlerShrubSeed, &vacuumiumShrubSeed, &silverShrubSeed, &quartzShrubSeed, &coal, &cheeseVineSeed, &topazVineSeed,
		&sapphireVineSeed, &leadVineSeed, &quartzVineSeed };
	
}

namespace Collectibles::Seeds
{
	// Vines
	Collectible copperShrubSeed = Collectible(Resources::Seeds::copperShrubSeed.Clone());
	Collectible ironShrubSeed = Collectible(Resources::Seeds::ironShrubSeed.Clone());
	Collectible rubyShrubSeed = Collectible(Resources::Seeds::rubyShrubSeed.Clone());
	Collectible emeraldShrubSeed = Collectible(Resources::Seeds::emeraldShrubSeed.Clone());
	Collectible rockShrubSeed = Collectible(Resources::Seeds::rockShrubSeed.Clone());
	Collectible shadeShrubSeed = Collectible(Resources::Seeds::shadeShrubSeed.Clone());
	Collectible bowlerShrubSeed = Collectible(Resources::Seeds::bowlerShrubSeed.Clone());
	Collectible vacuumiumShrubSeed = Collectible(Resources::Seeds::vacuumiumShrubSeed.Clone());
	Collectible silverShrubSeed = Collectible(Resources::Seeds::silverShrubSeed.Clone());
	Collectible quartzShrubSeed = Collectible(Resources::Seeds::quartzShrubSeed.Clone());
	Collectible coal = Collectible(Resources::Seeds::coal.Clone());

	// Vines
	Collectible cheeseVineSeed = Collectible(Resources::Seeds::cheeseVineSeed.Clone());
	Collectible topazVineSeed = Collectible(Resources::Seeds::topazVineSeed.Clone());
	Collectible sapphireVineSeed = Collectible(Resources::Seeds::sapphireVineSeed.Clone());
	Collectible leadVineSeed = Collectible(Resources::Seeds::leadVineSeed.Clone());
	Collectible quartzVineSeed = Collectible(Resources::Seeds::quartzVineSeed.Clone());

	// Keep a list of all of the seeds.
	vector<Collectible*> plantSeeds{ &copperShrubSeed, &ironShrubSeed, &rubyShrubSeed, &emeraldShrubSeed, &rockShrubSeed, &shadeShrubSeed,
		&bowlerShrubSeed, &vacuumiumShrubSeed, &silverShrubSeed, &quartzShrubSeed, &coal, &cheeseVineSeed, &topazVineSeed,
		&sapphireVineSeed, &leadVineSeed, &quartzVineSeed };
}

#pragma endregion

#pragma endregion


#pragma region Tower defense

EntityData turretData = EntityData(UPDATE::TURRET, VUPDATE::FRICTION, DUPDATE::TURRET, EDUPDATE::ENTITY, UIUPDATE::ENTITY, ONDEATH::LIGHTBLOCK);
class BasicTurret : public LightBlock
{
public:
	Projectile* projectile;
	float timeTill, timePer;

	BasicTurret(EntityData* data, Projectile* projectile, float timePer,
		JRGB lightColor, float range, float radius, RGBA color, RGBA color2, float mass, float bounciness, int maxHealth, int health, string name) :
		LightBlock(data, lightColor, true, range, vZero, radius, color, color2, mass, bounciness, maxHealth, health, name, PLAYER_A | PLANTS_A),
		projectile(projectile), timePer(timePer), timeTill(timePer)
	{ }

	BasicTurret(BasicTurret* baseClass, Vec3 pos, Vec3 dir, Entity* creator) :
		BasicTurret(*baseClass)
	{
		this->pos = pos;
		this->dir = dir;
		this->creator = creator;
		allegiance = creator->allegiance;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir, Entity* creator) override
	{
		return make_unique<Turret>(this, pos, dir, creator);
	}
};

#pragma endregion