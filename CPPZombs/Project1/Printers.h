#include "Defence.h"

namespace Shootables
{
	namespace Conveyers
	{
		PlacedOnLanding* duct = new PlacedOnLanding(Structures::Conveyers::duct);
	}
}

namespace Recipes
{
	RecipeA basicBullet(Costs::basicBullet, Projectiles::basicBullet);
	RecipeB copperWall(Costs::copperWall, Structures::Walls::copperWall);
	RecipeA duct(Costs::duct, Shootables::Conveyers::duct);
	RecipeA smallVacuum(Costs::smallVacuum, Shootables::smallVacuum);
	RecipeB largeVacuum(Costs::largeVacuum, Structures::Conveyers::largeVacuum);
	RecipeB basicTurret(Costs::turret, Structures::Defence::basicTurret);

	namespace PrinterRecipes
	{
		vector<RecipeA> smallPrinterRecipeAs{ basicBullet, duct, smallVacuum };
		vector<RecipeB> smallPrinterRecipeBs{ copperWall, largeVacuum, basicTurret };
	}
};

class Printer : public Duct
{
public:
	vector<RecipeA> recipeAs;
	vector<RecipeB> recipeBs;
	int currentRecipe = -1;
	Items items;

	Printer(vector<RecipeA> recipeAs, vector<RecipeB> recipeBs, float timePer, Vec2 pos = Vec2(0, 0), Vec2 dimensions = vOne, Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME"):
		Duct(timePer, pos, dimensions, color, mass, maxHealth, health, name), recipeAs(recipeAs), recipeBs(recipeBs)
	{ }

	Printer(Printer* baseClass, Vec2 dir, Vec2 pos) :
		Printer(*baseClass)
	{
		this->dir = dir;
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	Printer* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity * creator = nullptr) override
	{
		return new Printer(this, dir, pos);
	}

	void Update(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
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
		if (currentRecipe >= 0 && items.TryMake(currentRecipe < recipeAs.size() ? recipeAs[currentRecipe].first : recipeBs[currentRecipe - recipeAs.size()].first))
		{
			if (currentRecipe < recipeAs.size())
				entities->push_back(new Collectible(*recipeAs[currentRecipe].second, pos + dir));
			 else
				entities->push_back(recipeBs[currentRecipe - recipeAs.size()].second->Clone(pos + dir, dir, this));
		}
		else
			return false;
		return true;
	}

	Vec2 TopLeft() override
	{
		return FunctionalBlock::TopLeft() + Vec2(0, -9);
	}

	Vec2 BottomRight() override
	{
		return TopLeft() + Vec2((int)fmaxf(name.length() * 8.0f, (recipeAs.size() + recipeBs.size()) * 3.0f + 2), 12);
	}

	void UIUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		DrawUIBox(game, TopLeft(), BottomRight(), name, color);

		for (int i = 0; i < recipeAs.size(); i++)
			game->Draw(ToRSpace(pos) + Vec2(3 * (i + 2) + 1, 1), recipeAs[i].second->color);

		Vec2 tilePos = pos + Vec2(2 + static_cast<int>(recipeAs.size()), 0);
		for (int i = 0; i < recipeBs.size(); i++)
		{
			recipeBs[i].second->Draw(tilePos, recipeBs[i].second->color, game, entities, frameCount, inputs, dTime, dir);
			tilePos.x += 1;
		}
		if (currentRecipe >= 0)
		{
			Vec2 tl = FunctionalBlock::TopLeft() + Vec2(3 + currentRecipe, -1);
			game->Draw(tl, olc::BLACK);
		}
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 topLeft = TopLeft();
		Vec2 bottomRight = BottomRight();
		if (screenSpacePos.y <= bottomRight.y && screenSpacePos.y >= bottomRight.y - 4)
			currentRecipe = (screenSpacePos.x - topLeft.x - 3);
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}
};

namespace Structures
{
	namespace Printers
	{
		Printer* smallPrinter = new Printer(Recipes::PrinterRecipes::smallPrinterRecipeAs,
			Recipes::PrinterRecipes::smallPrinterRecipeBs, 3.0f, vZero, vOne, Color(74, 99, 67), 1, 10, 10, "Printer");
	}
}

namespace Shootables
{
	PlacedOnLanding* smallPrinter = new PlacedOnLanding(Structures::Printers::smallPrinter, "Printer", Structures::Printers::smallPrinter->color, 0);
	Collectible* cSmallPrinter = new Collectible(*smallPrinter);
}