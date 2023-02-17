#include "Game2.h"

class Temp
{
public:
	void Test()
	{

	}
};

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	printf("Greetings universe!\n");

	for (int i = 0; i < Plants::plants.size(); i++)
		Plants::plants[i]->seed = Collectibles::Seeds::plantSeeds[i];

#pragma region Update functions
	using namespace Updates;
	updates = { &EntityU, &FadeOutU, &ExplodeNextFrameU, &FadeOutPuddleU };
	using namespace DUpdates;
	dUpdates = { &EntityDU, &FadeOutDU, &FadeOutPuddleDU, &FadeOutGlowDU };
	/*
enum UPDATE
{
	ENTITY, FADEOUT, EXPLODENEXTFRAME, FADEOUTPUDDLE, PROJECTILE, FUNCTIONALBLOCK, FUNCTIONALBLOCK2, ENEMY, POUNCERSNAKE, SPIDER, POUNCER
};

vector<function<void()>> updates;

enum DUPDATE
{
	ENTITY, FADEOUT, FADEOUTPUDDLE, FADEOUTGLOW, DTOCOL, TREE, DECEIVER, PARENT, EXPLODER, COLORCYCLER, CAT
};

vector<function<void()>> dUpdates;

enum EDUPDATE // EDUPDATE = early dupdate = early draw update
{
	ENTITY, SPIDER
};

vector<function<void()>> eDUpdates;

enum UIUPDATE
{
	ENTITY, TREE, VINE
};*/

#pragma endregion

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}