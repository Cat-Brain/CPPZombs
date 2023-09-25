#include "Game2.h"

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	printf("Greetings universe!\n");
	
	for (int i = 0; i < Livestock::livestocks.size(); i++)
		Livestock::livestocks[i]->birthEntity = Livestock::livestockBirths[i];

#pragma region Virtual functions
#pragma region Update functions
	{
		using namespace Updates;
		using namespace Enemies::Updates;
		using namespace Livestock::Updates;
		updates = { EntityU, FadeOutU, CollectibleU, ExplodeNextFrameU, UpExplodeNextFrameU, FadeOutPuddleU, VacuumeForU, ProjectileU,
			FunctionalBlockU, FunctionalBlock2U, VineU, BasicTurretU, CircleTurretU, LaserTurretU, EnemyU, VacuumerU, SpiderU, CenticrawlerU, PouncerU, CatU, CataclysmU, EggU,
			KiwiU, PlayerU, FlareU, GrenadeU, FlameGlobU, FlamePuddleU, EngTurretU, BaseU };
	}
	{
		using namespace VUpdates;
		using namespace Enemies::VUpdates;
		vUpdates = { EntityVU, AirResistanceVU, GravityVU, GravityTrueVU, FrictionVU, ProjectileVU, VineVU, SnakeVU, SnakeConnectedVU, SpiderVU, CenticrawlerVU, ThiefVU };
	}
	{
		using namespace DUpdates;
		using namespace Enemies::DUpdates;
		using namespace Livestock::DUpdates;
		dUpdates = { EntityDU, FadeOutDU, CollectibleDU, FadeOutPuddleDU, FadeOutGlowDU, ProjectileDU, DToColDU, ShrubDU, TreeDU, BasicTurretDU, CircleTurretDU,
			LaserTurretDU, DeceiverDU, ParentDU, ExploderDU,
			SnakeConnectedDU, ColorCyclerDU, PouncerDU, CatDU, CataclysmDU, TankDU, LaserTankDU, KiwiDU, PlayerDU, EngTurretDU };
	}
	{
		using namespace UIUpdates;
		using namespace Enemies::UIUpdates;
		uiUpdates = { EntityUIU, ShrubUIU, TreeUIU, VineUIU, EnemyUIU, SnakeConnectedUIU, PlayerUIU, FlareUIU, EngineerUIU };
	}

	{
		using namespace TUpdates;
		tUpdates = { DefaultTU, ShrubTU, TreeTU, VineTU };
	}

	{
		using namespace Enemies::MUpdates;
		Enemies::mUpdates = { DefaultMU, SnakeMU, PouncerSnakeMU, SnakeConnectedMU, VacuumerMU, CenticrawlerMU, PouncerMU, CatMU, BaseTankMU, ThiefMU };
	}
	{
		using namespace Enemies::AUpdates;
		Enemies::aUpdates = { DefaultAU, ExploderAU, BoomcatAU, TankAU, MortarTankAU, GenericTankAU, LaserTankAU, ThiefAU };
	}

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
		
		using namespace OnDeaths;
		using namespace Enemies::OnDeaths;
		using namespace Livestock::OnDeaths;
		onDeaths = { EntityOD, FadeOutGlowOD, ShotItemOD, LightBlockOD, LightTowerOD, EnemyOD, ParentOD, ExploderOD, SnakeOD, PouncerSnakeOD,
			SnakeConnectedOD, VacuumerOD, SpiderOD, CenticrawlerOD, ThiefOD, KiwiOD, PlayerOD, BaseOD };

		using namespace ItemUs;
		itemUs = { ItemU, WaveModifierU };

		using namespace ItemODs;
		itemODs = { ItemOD, GoneOnLandItemOD, PlacedOnLandingOD, CorruptOnKillOD, PlacedOnLandingBoomOD, ExplodeOnLandingOD,
			UpExplodeOnLandingOD, ImproveSoilOnLandingOD, SetTileOnLandingOD,
			FlareFlameOD };
#pragma endregion

	game = make_unique<Game>();
	game->Construct();

	printf("\nFairwell universe!\n");
	return 0;
}