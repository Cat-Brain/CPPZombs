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

	Printer(vector<Recipe> recipes, Vec2 dir, float timePer, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME"):
		Duct(dir, timePer, pos, color, mass, maxHealth, health, name), recipes(recipes)
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
};

namespace Structures
{
	namespace Printers
	{
		Printer* smallPrinter = new Printer(Recipes::PrinterRecipes::smallPrinterRecipes, up, 30.0f, vZero, Color(74, 99, 67), 1, 10, 10, "Printer");
	}
}

namespace Shootables
{
	Item* smallPrinter = new PlacedOnLanding(Structures::Printers::smallPrinter, "Printer", Structures::Printers::smallPrinter->color, 0);
	Collectible* cSmallPrinter = new Collectible(*smallPrinter);
}