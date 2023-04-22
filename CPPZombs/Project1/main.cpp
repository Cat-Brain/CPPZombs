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

	for (int i = 0; i < Livestock::livestocks.size(); i++)
		Livestock::livestocks[i]->birthEntity = Livestock::livestockBirths[i];

#pragma region Virtaul functions
#pragma region Update functions
	{
		using namespace Updates;
		using namespace Enemies::Updates;
		using namespace Livestock::Updates;
		updates = { EntityU, FadeOutU, ExplodeNextFrameU, UpExplodeNextFrameU, FadeOutPuddleU, VacuumeForU, ProjectileU, FunctionalBlockU, FunctionalBlock2U,
		EnemyU, PouncerSnakeU, VacuumerU, SpiderU, CenticrawlerU, PouncerU, CatU, CataclysmU, EggU, KiwiU, PlayerU, GrenadeU };
	}
	{
		using namespace VUpdates;
		vUpdates = { EntityVU, FrictionVU };
	}
	{
		using namespace DUpdates;
		using namespace Enemies::DUpdates;
		using namespace Livestock::DUpdates;
		dUpdates = { EntityDU, FadeOutDU, FadeOutPuddleDU, FadeOutGlowDU, DToColDU, TreeDU, DeceiverDU, ParentDU, ExploderDU, SnakeConnectedDU, ColorCyclerDU,
		PouncerDU, CatDU, CataclysmDU, TankDU, KiwiDU, PlayerDU };
	}
	{
		using namespace EDUpdates;
		using namespace Enemies::EDUpdates;
		eDUpdates = { EntityEDU, SnakeEDU, SnakeConnectedEDU, SpiderEDU };
	}
	{
		using namespace UIUpdates;
		using namespace Enemies::UIUpdates;
		uiUpdates = { EntityUIU, TreeUIU, VineUIU, EnemyUIU, SnakeConnectedUIU };
	}

	{
		using namespace TUpdates;
		tUpdates = { DefaultTU, TreeTU, VineTU };
	}

	{
		using namespace Enemies::MUpdates;
		Enemies::mUpdates = { DefaultMU, SnakeMU, PouncerSnakeMU, SnakeConnectedMU, VacuumerMU, CenticrawlerMU, PouncerMU, CatMU, TankMU };
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
		secondaries = { GrenadeThrow, TornadoSpin };
	}
	{
		using namespace Utilities;
		utilities = { TacticoolRoll, MightyShove };
	}
	#pragma endregion
		
		using namespace OnDeaths;
		using namespace Enemies::OnDeaths;
		using namespace Livestock::OnDeaths;
		onDeaths = { EntityOD, FadeOutGlowOD, ShotItemOD, LightBlockOD, VineOD, EnemyOD, ParentOD, ExploderOD, SnakeOD, PouncerSnakeOD,
			SnakeConnectedOD, VacuumerOD, SpiderOD, CenticrawlerOD, KiwiOD, PlayerOD };

		using namespace OverlapFuns;
		overlapFuns = { EntityOF };

		using namespace ItemUs;
		itemUs = { ItemU, WaveModifierU };

		using namespace ItemODs;
		itemODs = { ItemOD, GoneOnLandItemOD, PlacedOnLandingOD, CorruptOnKillOD, ExplodeOnLandingOD, UpExplodeOnLandingOD, ImproveSoilOnLandingOD, SetTileOnLandingOD };
	
#pragma endregion

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}