#include "Game2.h"

void FuncA()
{
	float sum = 0;
	for (int i = 0; i < 100; i++)
		sum += sqrtf(i);
	printf("%f, ", sum);
}

void FuncB()
{
	float sum = 0;
	for (int i = 1; i < 100; i++)
		sum += 1.f / i;
	printf("%f, ", sum);
}

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	printf("Greetings universe!\n");

	/*vector<function<void()>> testFuncs{FuncA, FuncB};
	std::chrono::steady_clock::time_point startTest, endTest;

	startTest = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 1000; i++)
		testFuncs[i % 2]();
	endTest = std::chrono::high_resolution_clock::now();
	printf("\n\n%i\n\n", (int)std::chrono::duration_cast<std::chrono::microseconds>(endTest - startTest).count());

	startTest = std::chrono::high_resolution_clock::now();
	#pragma omp parallel for (int i = 0; i < 1000; i++) testFuncs[i % 2]();
	endTest = std::chrono::high_resolution_clock::now();
	printf("\n\n%i\n\n", (int)std::chrono::duration_cast<std::chrono::microseconds>(endTest - startTest).count());
	#pragma omp parallel
	{
		printf("!\n");
	}*/
	
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

/*
v0.7.6.0 Changelog:
	Decreased oppacity of vaccumium
	Some reorganization, refactoring, and stuff of that kin
	Added max round spawns for enemies! Maybe will help performance?
	Reimplemented wave counter as now being time based. Not sure why that wasn't the case...
	Changed "wave" that enemies spawn on for most enemies
*/