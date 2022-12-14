#include "Defence.h"

namespace Recipes
{
	Recipe basicBullet(Costs::basicBullet, Projectiles::basicBullet);
	Recipe copperWall(Costs::copperWall, Structures::Walls::copperWall);
	Recipe duct(Costs::duct, Structures::Conveyers::duct);
	Recipe smallVacuum(Costs::smallVacuum, Structures::Conveyers::smallVacuum);
	Recipe largeVacuum(Costs::largeVacuum, Structures::Conveyers::largeVacuum);
	Recipe turret(Costs::turret, Projectiles::basicBullet);

	namespace PrinterRecipes
	{
		vector<Recipe> smallPrinterRecipes{ basicBullet, copperWall, duct, smallVacuum, largeVacuum, turret };
	}
};

class Printer : public Duct
{
public:
	vector<Recipe> recipes;
	int currentRecipe = -1;
	Items items;

	Printer(vector<Recipe> recipes , float timePer, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME"):
		Duct(timePer, pos, color, mass, maxHealth, health, name), recipes(recipes)
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

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
	{
		vector<Collectible*> newCollectibles = CollectiblesAtEPos(pos, ((Entities*)entities)->collectibles);
		for (Collectible* collectible : newCollectibles)
		{
			items.push_back(collectible->baseClass);
			((Entities*)entities)->Remove(collectible);
		}

		Duct::Update(screen, entities, frameCount, inputs, dTime);
	}

	void TUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		if(currentRecipe != -1 && items.TryMake(recipes[currentRecipe].first))
			entities->push_back(recipes[currentRecipe].second);
	}

	Vec2 TopLeft() override
	{
		return ToRSpace(pos) + Vec2(3, 0);
	}

	Vec2 BottomRight() override
	{
		return TopLeft() + Vec2((int)fmaxf(name.length() * 8.0f, recipes.size() * 3.0f), 12);
	}

	void UIUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
	{
		DrawUIBox(screen, TopLeft(), BottomRight(), name, color);
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 topLeft = ToRSpace(pos) + Vec2(3, 0);
		Vec2 bottomRight = topLeft + Vec2((int)name.length() * 8, 8);
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}
};

namespace Structures
{
	namespace Printers
	{
		Printer* smallPrinter = new Printer(Recipes::PrinterRecipes::smallPrinterRecipes, 30.0f, vZero, Color(74, 99, 67), 1, 10, 10, "Printer");
	}
}

namespace Shootables
{
	PlacedOnLanding* smallPrinter = new PlacedOnLanding(Structures::Printers::smallPrinter, "Printer", Structures::Printers::smallPrinter->color, 0);
	Collectible* cSmallPrinter = new Collectible(*smallPrinter);
}