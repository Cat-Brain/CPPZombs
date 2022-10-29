#include <stdio.h>
#include "Game.h"

int main()
{
	printf("Hello!\nThis game was made using the one lone coder pixel game engine\n");

	Game game;
	if (game.Construct(16, 16, 16, 16))
		game.Start();

	printf("Goodbye!\n");
}