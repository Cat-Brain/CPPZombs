#include "Defence.h"

namespace Recipes
{
	Recipe basicBullet(Costs::basicBullet, Projectiles::basicBullet);
	Recipe copperWall(Costs::copperWall, Structures::Walls::copperWall);
	Recipe duct(Costs::duct, Structures::Conveyers::duct);
	Recipe vacuum(Costs::vacuum, Structures::Conveyers::vacuum);
	Recipe largeVacuum(Costs::largeVacuum, Structures::Conveyers::largeVacuum);
	Recipe turret(Costs::turret, Projectiles::basicBullet);

	namespace PrinterRecipes
	{
		vector<Recipe> smallPrinterRecipes{ basicBullet, copperWall };
	}
};

class Printer : public FunctionalBlock
{
public:
	vector<Recipe> recipes;

	Printer(vector<Recipe> recipes, float timePer, Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME"):
		FunctionalBlock(timePer, pos, color, mass, maxHealth, health, name), recipes(recipes)
	{ }
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
	Item* smallPrinter = new PlacedOnLanding(Structures::Printers::smallPrinter, "Printer", Color(74, 99, 67), 0);
	Collectible* cSmallPrinter = new Collectible(smallPrinter->Clone());
}