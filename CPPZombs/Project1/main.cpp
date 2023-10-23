#include "Game2.h"

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	printf("Greetings universe!\n");
	
	for (int i = 0; i < Livestock::livestocks.size(); i++)
		Livestock::livestocks[i]->birthEntity = Livestock::livestockBirths[i];

#pragma region Game Stuff
	{
		using namespace StartCallbacks;
		startCallbacks = { Default, Tutorial1, Tutorial2, Tutorial3 };
	}
	{
		using namespace UpdateModes;
		updateModes = { MainMenu, CharSelect, SeedSelect, TutorialSelect, InGame, Paused, Dead };
	}
	{
		using namespace PreUpdates;
		preUpdates = { Default, Tutorial1, Tutorial2, Tutorial3 };
	}
	{
		using namespace PostUpdates;
		postUpdates = { Default, Tutorial1, Tutorial2, Tutorial3 };
	}
#pragma endregion

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}