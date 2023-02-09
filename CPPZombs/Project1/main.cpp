#include "Game2.h"

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	printf("Greetings universe!\n");

	for (int i = 0; i < Plants::plants.size(); i++)
		Plants::plants[i]->seed = Collectibles::Seeds::plantSeeds[i];

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}