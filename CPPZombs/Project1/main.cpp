#include "Game.h"

int main()
{
	Game game;
	if (game.Construct(screenWidth * 3, screenHeight * 3, 8, 8, true))
		game.Start();
	return 0;
}