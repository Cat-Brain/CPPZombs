#include "Game.h"

int main()
{
	Game game;
	if (game.Construct(screenWidth, screenHeight, 8, 8, false))
		game.Start();
	return 0;
}