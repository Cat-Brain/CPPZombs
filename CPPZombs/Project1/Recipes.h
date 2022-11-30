#include "Enemy.h"

typedef pair<Cost, Entity*> Recipe;

namespace Recipes
{
	Recipe basicBullet(Costs::basicBullet, basicBullet);
};

typedef pair<Recipe, bool> rPair;

class AllRecipes : public vector<rPair>
{
public: 
	using vector<rPair>::vector;

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime)
	{

	}

	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime)
	{

	}
};

AllRecipes recipes{ rPair(Recipes::basicBullet, false), rPair(Recipes::copperWall, false), rPair(Recipes::conveyer, false),
	rPair(Recipes::vacuum, false), rPair(Recipes::largeVacuum, false), rPair(Recipes::turret, false) };