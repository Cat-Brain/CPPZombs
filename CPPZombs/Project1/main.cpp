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

#pragma region Virtaul functions
	{
	#pragma region Update functions
		using namespace Updates;
		using namespace Enemies::Updates;
		updates = { EntityU, FadeOutU, ExplodeNextFrameU, FadeOutPuddleU, VacuumeForU, ProjectileU, FunctionalBlockU, FunctionalBlock2U,
		EnemyU, PouncerSnakeU, VacuumerU, SpiderU, CenticrawlerU, PouncerU, CatU, CataclysmU, PlayerU };
		using namespace DUpdates;
		using namespace Enemies::DUpdates;
		dUpdates = { EntityDU, FadeOutDU, FadeOutPuddleDU, FadeOutGlowDU, DToColDU, TreeDU, DeceiverDU, ParentDU, ExploderDU, ColorCyclerDU,
		CatDU, CataclysmDU };
		using namespace EDUpdates;
		eDUpdates = { EntityEDU };
		using namespace UIUpdates;
		using namespace Enemies::UIUpdates;
		uiUpdates = { EntityUIU, TreeUIU, VineUIU, EnemyUIU };

		using namespace TUpdates;
		tUpdates = { DefaultTU, TreeTU, VineTU };

		using namespace Enemies::MUpdates;
		Enemies::mUpdates = { DefaultMU, SnakeMU, PouncerSnakeMU, VacuumerMU, CenticrawlerMU, PouncerMU, TankMU };
		using namespace Enemies::AUpdates;
		Enemies::aUpdates = { DefaultAU, ExploderAU, BoomcatAU, TankAU };
	#pragma endregion
		
		using namespace OnDeaths;
		using namespace Enemies::OnDeaths;
		onDeaths = { EntityOD, FadeOutGlowOD, ShotItemOD, LightBlockOD, VineOD, EnemyOD, ParentOD, ExploderOD, SnakeOD, PouncerSnakeOD, VacuumerOD,
		SpiderOD, CenticrawlerOD, PlayerOD };

		using namespace OverlapFuns;
		overlapFuns = { EntityOF };

		using namespace ItemODs;
		itemODs = { ItemOD, GoneOnLandItemOD, PlacedOnLandingOD, CorruptOnKillOD, ExplodeOnLandingOD };
	}
	
#pragma endregion

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}