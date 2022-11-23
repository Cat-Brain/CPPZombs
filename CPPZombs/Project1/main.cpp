#include "Game.h"

int main()
{
	printf("Greetings universe!\n");

	#pragma region Trees

	copperTree->seed = cCopperTreeSeed;
	ironTree->seed = cIronTreeSeed;

	#pragma endregion

	Game game;
	if (game.Construct(screenWidth * 3, screenHeight * 3, 8, 8, true))
		game.Start();

	printf("\nFairwell universe!\n");
	return 0;
}