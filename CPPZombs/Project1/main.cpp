#include "Game2.h"

int main()
{
	printf("Greetings universe!\nDo you want to use fullscreen? 'y' or 'n'");

	#pragma region Trees

	for (int i = 0; i < Plants::plants.size(); i++)
		Plants::plants[i]->seed = Collectibles::Seeds::plantSeeds[i];

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