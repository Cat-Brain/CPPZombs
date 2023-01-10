#include "Game2.h"

int main()
{
	printf("Greetings universe!\n");

	#pragma region Trees

	copperTree->seed = cCopperTreeSeed;
	ironTree->seed = cIronTreeSeed;
	rubyTree->seed = cRubyTreeSeed;
	emeraldTree->seed = cEmeraldTreeSeed;
	cheeseTree->seed = cCheeseTreeSeed;

	#pragma endregion

	Game game;
	if (game.Construct(screenWidth * 4, screenHeight * 4, 2, 2, true))
		game.Start();

	printf("\nFairwell universe!\n");
	return 0;
}