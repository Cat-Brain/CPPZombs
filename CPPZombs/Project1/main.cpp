#include "Game2.h"

int main()
{
	printf("Greetings universe!\nDo you want to use fullscreen? 'y' or 'n'");

	#pragma region Trees

	Trees::copperTree->seed = Collectibles::Seeds::copperTreeSeed;
	Trees::ironTree->seed = Collectibles::Seeds::ironTreeSeed;
	Trees::rubyTree->seed = Collectibles::Seeds::rubyTreeSeed;
	Trees::emeraldTree->seed = Collectibles::Seeds::emeraldTreeSeed;
	Trees::cheeseTree->seed = Collectibles::Seeds::cheeseTreeSeed;

	#pragma endregion

	string fullscreen;
	std::cin >> fullscreen;
	while (fullscreen != "y" && fullscreen != "n")
		std::cin >> fullscreen;

	Game game;
	if (game.Construct(screenWidth * 4, screenHeight * 4, 2, 2, fullscreen == "y", true))
		game.Start();

	printf("\nFairwell universe!\n");
	return 0;
}