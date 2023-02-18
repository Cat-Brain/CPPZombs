#include "Game2.h"

class Temp
{
public:
	void Test()
	{

	}
};

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	printf("Greetings universe!\n");

	for (int i = 0; i < Plants::plants.size(); i++)
		Plants::plants[i]->seed = Collectibles::Seeds::plantSeeds[i];

#pragma region Update functions
	using namespace Updates;
	using namespace Enemies::Updates;
	updates = { EntityU, FadeOutU, ExplodeNextFrameU, FadeOutPuddleU, ProjectileU, FunctionalBlockU, FunctionalBlock2U,
	EnemyU, PouncerSnakeU, SpiderU, PouncerU, PlayerU };
	using namespace DUpdates;
	using namespace Enemies::DUpdates;
	dUpdates = { EntityDU, FadeOutDU, FadeOutPuddleDU, FadeOutGlowDU, DToColDU, TreeDU, DeceiverDU, ParentDU, ExploderDU, ColorCyclerDU,
	CatDU };
	using namespace EDUpdates;
	using namespace Enemies::EDUpdates;
	eDUpdates = { EntityEDU, SpiderEDU };
	using namespace UIUpdates;
	using namespace Enemies::UIUpdates;
	uiUpdates = { EntityUIU, TreeUIU, VineUIU, EnemyUIU };

	using namespace TUpdates;
	tUpdates = { DefaultTU, TreeTU, VineTU };

	using namespace Enemies::MUpdates;
	Enemies::mUpdates = { DefaultMU, SnakeMU, PouncerSnakeMU, VacuumerMU, PouncerMU, TankMU };
	using namespace Enemies::AUpdates;
	Enemies::aUpdates = { DefaultAU, ExploderAU, VacuumerAU, RangerAU, BoomcatAU, TankAU };
#pragma endregion

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}