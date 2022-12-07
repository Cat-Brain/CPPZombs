#include "Enemy.h"

namespace Recipes
{
	Recipe basicBullet(Costs::basicBullet, Projectiles::basicBullet);
	Recipe copperWall(Costs::copperWall, Placeables::copperWall);
	Recipe duct(Costs::duct, Structures::Conveyers::duct);
	Recipe vacuum(Costs::basicBullet, Projectiles::basicBullet);
	Recipe largeVacuum(Costs::basicBullet, Projectiles::basicBullet);
	Recipe turret(Costs::basicBullet, Projectiles::basicBullet);
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
		using namespace Recipes;
		Printer* smallPrinter = new Printer({basicBullet}, 30.0f, vZero, Color(74, 99, 67), 1, 10, 10, "Printer");
	}
}

namespace Shootables
{
	Item* printer = new PlacedOnLanding(Structures::Printers::smallPrinter, "Printer", olc::DARK_BLUE, 0);
}