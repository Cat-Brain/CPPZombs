#include "Enemy.h"

namespace Shootables
{
	Item* printer = new PlacedOnLanding(Structures::printer, "Printer", olc::DARK_BLUE, 0);
}

typedef pair<Cost, Entity*> Recipe;

namespace Recipes
{
	Recipe basicBullet(Costs::basicBullet, Projectiles::basicBullet);
	Recipe copperWall(Costs::copperWall, Placeables::copperWall);
	Recipe duct(Costs::duct, Structures::Conveyers::duct);
	Recipe vacuum(Costs::basicBullet, Projectiles::basicBullet);
	Recipe largeVacuum(Costs::basicBullet, Projectiles::basicBullet);
	Recipe turret(Costs::basicBullet, Projectiles::basicBullet);
};

typedef pair<Recipe, bool> RPair;

class AllRecipes : public vector<RPair>
{
public: 
	using vector<RPair>::vector;

	int index = 0;

	void Update(Screen* screen, Entities* entities, int frameCount, Inputs inputs, float dTime)
	{

	}

	void DUpdate(Screen* screen, Entities* entities, int frameCount, Inputs inputs, float dTime)
	{
		Vec2 topLeft = playerPos + Vec2(-screenWidthH / 2, screenHeightH / 2);
		for (int i = 0; i < size(); i++)
		{
			((*this)[i]).first.second->Draw(topLeft + Vec2(i % 2, i / 2), olc::WHITE, screen, entities, frameCount, inputs, dTime);
		}
	}
};

AllRecipes recipes{ RPair(Recipes::basicBullet, true), RPair(Recipes::copperWall, true), RPair(Recipes::duct, true),
	RPair(Recipes::vacuum, true), RPair(Recipes::largeVacuum, true), RPair(Recipes::turret, true) };