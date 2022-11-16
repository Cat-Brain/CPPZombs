#include "Game.h"

int main()
{
	printf("Greetings universe!\n");
	Game game;
	if (game.Construct(screenWidth * 3, screenHeight * 3, 8, 8, false))
		game.Start();
	printf("\nFairwell universe!\n");
	return 0;
}