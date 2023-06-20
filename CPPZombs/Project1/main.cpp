#include "Game2.h"

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	printf("Greetings universe!\n");
	
	for (int i = 0; i < Livestock::livestocks.size(); i++)
		Livestock::livestocks[i]->birthEntity = Livestock::livestockBirths[i];

#pragma region Virtaul functions
#pragma region Update functions
	{
		using namespace Updates;
		using namespace Enemies::Updates;
		using namespace Livestock::Updates;
		updates = { EntityU, FadeOutU, CollectibleU, ExplodeNextFrameU, UpExplodeNextFrameU, FadeOutPuddleU, VacuumeForU, ProjectileU,
			FunctionalBlockU, FunctionalBlock2U, VineU, BasicTurretU, EnemyU, VacuumerU, SpiderU, CenticrawlerU, PouncerU, CatU, CataclysmU, EggU,
			KiwiU, PlayerU, TurretU, RoverU, EngineerU, GrenadeU };
	}
	{
		using namespace VUpdates;
		using namespace Enemies::VUpdates;
		vUpdates = { EntityVU, AirResistanceVU, FrictionVU, VineVU, SnakeVU, SnakeConnectedVU, SpiderVU };
	}
	{
		using namespace DUpdates;
		using namespace Enemies::DUpdates;
		using namespace Livestock::DUpdates;
		dUpdates = { EntityDU, FadeOutDU, FadeOutPuddleDU, FadeOutGlowDU, ProjectileDU, DToColDU, ShrubDU, TreeDU, BasicTurretDU, DeceiverDU, ParentDU, ExploderDU,
			SnakeConnectedDU, ColorCyclerDU, PouncerDU, CatDU, CataclysmDU, TankDU, KiwiDU, PlayerDU, TurretDU, RoverDU };
	}
	{
		using namespace UIUpdates;
		using namespace Enemies::UIUpdates;
		uiUpdates = { EntityUIU, ShrubUIU, TreeUIU, VineUIU, EnemyUIU, SnakeConnectedUIU, PlayerUIU, EngineerUIU };
	}

	{
		using namespace TUpdates;
		tUpdates = { DefaultTU, ShrubTU, TreeTU, VineTU };
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
		pMovements = { Default, Jetpack };
	}
	{
		using namespace Primaries;
		primaries = { Slingshot, EngShoot, CircleGun };
	}
	{
		using namespace Secondaries;
		secondaries = { GrenadeThrow, TornadoSpin, EngModeUse };
	}
	{
		using namespace Utilities;
		utilities = { TacticoolRoll, MightyShove, EngModeSwap };
	}
	#pragma endregion
		
		using namespace OnDeaths;
		using namespace Enemies::OnDeaths;
		using namespace Livestock::OnDeaths;
		onDeaths = { EntityOD, FadeOutGlowOD, ShotItemOD, LightBlockOD, LightTowerOD, EnemyOD, ParentOD, ExploderOD, SnakeOD, PouncerSnakeOD,
			SnakeConnectedOD, VacuumerOD, SpiderOD, CenticrawlerOD, KiwiOD, PlayerOD, BaseOD };

		using namespace ItemUs;
		itemUs = { ItemU, WaveModifierU };

		using namespace ItemODs;
		itemODs = { ItemOD, GoneOnLandItemOD, PlacedOnLandingOD, CorruptOnKillOD, PlacedOnLandingBoomOD, ExplodeOnLandingOD,
			UpExplodeOnLandingOD, ImproveSoilOnLandingOD, SetTileOnLandingOD };
#pragma endregion

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}