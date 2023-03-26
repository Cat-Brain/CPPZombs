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
#pragma region Update functions
	{
		using namespace Updates;
		using namespace Enemies::Updates;
		updates = { EntityU, FadeOutU, ExplodeNextFrameU, FadeOutPuddleU, VacuumeForU, ProjectileU, FunctionalBlockU, FunctionalBlock2U,
		EnemyU, PouncerSnakeU, VacuumerU, SpiderU, CenticrawlerU, PouncerU, CatU, CataclysmU, PlayerU };
	}
	{
		using namespace VUpdates;
		vUpdates = { EntityVU, FrictionVU };
	}
	{
		using namespace DUpdates;
		using namespace Enemies::DUpdates;
		dUpdates = { EntityDU, FadeOutDU, FadeOutPuddleDU, FadeOutGlowDU, DToColDU, TreeDU, DeceiverDU, ParentDU, ExploderDU, SnakeDU, ColorCyclerDU,
		PouncerDU, CatDU, CataclysmDU, PlayerDU };
	}
	{
		using namespace EDUpdates;
		using namespace Enemies::EDUpdates;
		eDUpdates = { EntityEDU, SnakeEDU, SpiderEDU };
	}
	{
		using namespace UIUpdates;
		using namespace Enemies::UIUpdates;
		uiUpdates = { EntityUIU, TreeUIU, VineUIU, EnemyUIU };
	}

	{
		using namespace TUpdates;
		tUpdates = { DefaultTU, TreeTU, VineTU };
	}

	{
		using namespace Enemies::MUpdates;
		Enemies::mUpdates = { DefaultMU, SnakeMU, PouncerSnakeMU, VacuumerMU, CenticrawlerMU, PouncerMU, CatMU, TankMU };
	}
	{
		using namespace Enemies::AUpdates;
		Enemies::aUpdates = { DefaultAU, ExploderAU, BoomcatAU, TankAU };
	}
	{
		using namespace PMovements;
		pMovements = { Default };
	}
	{
		using namespace Primaries;
		primaries = { Slingshot, CircleGun };
	}
	{
		using namespace Secondaries;
		secondaries = { Bayonet, TornadoSpin };
	}
	{
		using namespace Utilities;
		utilities = { TacticoolRoll, MightyShove };
	}
	#pragma endregion
		
		using namespace OnDeaths;
		using namespace Enemies::OnDeaths;
		onDeaths = { EntityOD, FadeOutGlowOD, ShotItemOD, LightBlockOD, VineOD, EnemyOD, ParentOD, ExploderOD, SnakeOD, PouncerSnakeOD, VacuumerOD,
		SpiderOD, CenticrawlerOD, PlayerOD };

		using namespace OverlapFuns;
		overlapFuns = { EntityOF };

		using namespace ItemUs;
		itemUs = { ItemU, WaveModifierU };

		using namespace ItemODs;
		itemODs = { ItemOD, GoneOnLandItemOD, PlacedOnLandingOD, CorruptOnKillOD, ExplodeOnLandingOD, ImproveSoilOnLandingOD, SetTileOnLandingOD };
	
#pragma endregion

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}