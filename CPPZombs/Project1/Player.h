#include "Livestock.h"

class UseableItem : public Item
{
public:
	UseableItem(ITEMTYPE type, ITEMU useFunction, string name = "NULL NAME", string typeName = "NULL TYPE NAME", int intType = 0, RGBA color = RGBA(), float useTime = 0) :
		Item(type, name, typeName, intType, color, 0, 0, useTime)
	{
		itemU = useFunction;
	}
};

namespace Resources
{
	UseableItem waveModifier = UseableItem(ITEMTYPE::WAVE_MODIFIER, ITEMU::WAVEMODIFIER, "Wave Modifier", "Special Item", 0, RGBA(194, 99, 21), 1.f);
}



class PlayerInventory : public Items
{
private:
	bool isHovering = false;
public:
	uint width; // How many per vertical slice. The amount shown when the inventory is closed.
	uint height; // How many per horizontal slice.
	int currentSelected = -1; // Currently selected item for item place swapping. -1 means nothing selected.
	int currentRow = 0; // ADD DESCRIPTION
	bool isOpen = false, isBuilding = false;
	Entity* player;
	vector<std::pair<int, ITEMTYPE>> oldPlacements;

	PlayerInventory(Entity* player = nullptr, uint width = 10, uint height = 4, Items startItems = Items()) :
		player(player), width(width), height(height), Items(width * height + 1, ItemInstance(ITEMTYPE::DITEM, 0))
	{
		for (int i = 0; i < startItems.size() && i < size(); i++)
			(*this)[i] = startItems[i];
	}

	ItemInstance GetCurrentItem() override
	{
		if (isBuilding)
			return bItem.Clone(0);
		if ((*this)[width * currentRow + currentIndex].count > 0)
			return (*this)[width * currentRow + currentIndex].Clone(1);
		return ItemInstance(ITEMTYPE::DITEM, 0);
	}

	virtual ItemInstance GetCurrentOffhand()
	{
		if (isBuilding)
			return bItem.Clone(0);
		if ((*this)[size() - 1].count > 0)
			return (*this)[size() - 1].Clone(1);
		return ItemInstance(ITEMTYPE::DITEM, 0);
	}

	byte TryTake(ItemInstance item) override
	{
		for (int i = 0; i < size(); i++)
		{
			if ((*this)[i] == item)
			{
				if ((*this)[i].count < item.count)
					return TRYTAKE::TO_FEW;
				if ((*this)[i].count != item.count)
				{
					(*this)[i].count -= item.count;
					return TRYTAKE::DECREMENTED;
				}
				(*this)[i] = ItemInstance(ITEMTYPE::DITEM, 0);
				return TRYTAKE::DELETED;
			}
		}
		return TRYTAKE::UNFOUND;
	}

	bool CanMake(Recipe cost) override
	{
		PlayerInventory clone = *this;
		for (ItemInstance item : cost)
			if (TRYTAKE::Failure(clone.TryTake(item)))
				return false;
		return true;
	}

	int CanMakeCount(Recipe cost)
	{
		int result = 0;
		PlayerInventory clone = *this;
		for (int i = 0; i < 100; i++)
		{
			for (ItemInstance item : cost)
				if (TRYTAKE::Failure(clone.TryTake(item)))
					return result;
			result++;
		}
		return 0;
	}

	bool TryMake(Recipe cost) override
	{
		PlayerInventory clone = *this;
		for (ItemInstance item : cost)
			if (TRYTAKE::Failure(clone.TryTake(item)))
				return false;
		*this = clone;
		return true;
	}

	bool RemoveIfEmpty(int index) // True means did remove, false means did not.
	{
		if ((*this)[index].count > 0)
			return false;

		oldPlacements.push_back({ index, (*this)[index].type });
		(*this)[index] = ItemInstance(ITEMTYPE::DITEM, 0);
		return true;
	}

	void push_back(ItemInstance instance)
	{
		for (int i = 0; i < size(); i++)
			if ((*this)[i] == instance)
			{
				(*this)[i].count += instance.count;
				return;
			}
		for (int i = 0; i < oldPlacements.size(); i++)
			if (oldPlacements[i].second == instance.type)
			{
				if ((*this)[oldPlacements[i].first].count == 0)
				{
					(*this)[oldPlacements[i].first] = instance;
					oldPlacements.erase(oldPlacements.begin() + i);
					return;
				}
				oldPlacements.erase(oldPlacements.begin() + i);
				break;
			}
		for (int i = 0, j = width * currentRow; i < size(); i++, j = (j + 1) % size())
			if ((*this)[j].count == 0)
			{
				(*this)[j] = instance;
				return;
			}
	}

	void Update(bool shouldScroll);

	void DUpdate()
	{
		int scrHeight = ScrHeight();
		float scale = scrHeight / (3.f * width), scale2 = scale * 0.2f;
		float tScale = 0.66666f / width, tScale2 = tScale * 0.2f;
		iVec2 offset = vZeroI2 - ScrDim();
		Vec2 tOffset = -vOne;
		if (isBuilding)
		{
#pragma region Crafting
			vector<Tower*> buildableTowers;
			for (Tower* tower : Defences::Towers::towers)
				if (CanMake(tower->recipe))
					buildableTowers.push_back(tower);
			if (buildableTowers.size() == 0)
				return font.Render("Nothing craftable", Vec2(-ScrWidth(), -ScrHeight()), scale * 2, RGBA(255, 255, 255));

			// Show which is currently selected
			game->DrawFBL(offset + iVec2(0, static_cast<int>(scale * currentIndex * 2)), RGBA(),
				Vec2(font.TextWidthTrue(buildableTowers[currentIndex]->name + " " + to_string(CanMakeCount(buildableTowers[currentIndex]->recipe))) * scale, scale));
			for (uint i = 0; i < buildableTowers.size(); i++)
			{
				font.Render(buildableTowers[i]->name + " " + to_string(CanMakeCount(buildableTowers[i]->recipe)),
					Vec2(-ScrWidth(), -ScrHeight() + scale * 2 * i), scale * 2, buildableTowers[i]->color);
			}
#pragma endregion
			return;
		}
		else if (isOpen)
		{
			game->DrawFBL(offset, RGBA(127, 127, 127, 127), Vec2(scale * height, scale * width));
			game->DrawFBL(Vec2(offset.x + currentRow * scale * 2, offset.y), RGBA(127, 127, 127, 127), Vec2(scale, scale * width));
			game->DrawFBL(Vec2(0, currentIndex * scale * 2.f) - Vec2(ScrDim()), RGBA(0, 0, 0, 127), Vec2(scale));

			game->DrawFBL(Vec2(offset.x + height * scale * 2, offset.y), RGBA(127, 127, 127, 127), Vec2(scale));
			if ((*this)[size() - 1].count != 0)
				game->DrawTextured(spriteSheet, (*this)[size() - 1]->intType, tOffset, Vec2(tScale * height, 0),
					(*this)[size() - 1]->color, Vec2(tScale));

			if (currentSelected != -1)
				game->DrawTextured(spriteSheet, (*this)[currentSelected]->intType, tOffset, 2.f * game->inputs.screenMousePosition / float(ScrHeight()),
					(*this)[currentSelected]->color, Vec2(tScale));

			if (isHovering)
			{
				iVec2 rPos = game->inputs.screenMousePosition / scale;
				game->DrawFBL(Vec2(rPos) * scale * 2.f - Vec2(ScrDim()), RGBA(), Vec2(scale));
				int currentHovered = rPos.x * width + rPos.y;
				if ((*this)[currentHovered].count != 0)
					font.Render((*this)[currentHovered]->name + "  " + to_string((*this)[currentHovered].count) + "  " + (*this)[currentHovered]->typeName,
						iVec2(game->inputs.screenMousePosition * 2.f) - ScrDim(), scale * 2, (*this)[currentHovered]->color);
			}
			if (currentSelected != -1)
				game->DrawFBL(offset + ToIV2(Vec2(currentSelected / width, currentSelected % width) * (scale * 2)), (*this)[currentSelected]->color, Vec2(scale));
			for (uint y = 0, i = 0; y < height; y++)
				for (uint x = 0; x < width; x++, i++)
				{
					if ((*this)[i].count == 0) continue;
					
					game->DrawTextured(spriteSheet, (*this)[i]->intType, tOffset, Vec2(tScale * y, tScale * x),
						(*this)[i]->color, Vec2(tScale));
				}
			return;
		}
		int indexOffset = width * currentRow;
		game->DrawFBL(offset + iVec2(0, static_cast<int>(scale * currentIndex * 2)), (*this)[currentIndex + indexOffset]->color, Vec2(scale));
		for (uint i = 0; i < width; i++)
		{
			if ((*this)[i + indexOffset].count == 0) continue;

			game->DrawTextured(spriteSheet, (*this)[i + indexOffset]->intType, tOffset, Vec2(0, tScale * i),
				i == currentIndex ? RGBA() : (*this)[i + indexOffset]->color, Vec2(tScale));
			font.Render(" " + (*this)[i + indexOffset]->name + "  " + to_string((*this)[i + indexOffset].count) + "  " + (*this)[i + indexOffset]->typeName,
				Vec2(-ScrWidth() + scale * 2, -ScrHeight() + scale * 2 * i), scale * 2, (*this)[i + indexOffset]->color);
		}

		if ((*this)[size() - 1].count != 0)
		{
			game->DrawTextured(spriteSheet, (*this)[size() - 1]->intType, tOffset, Vec2(0, tScale * width),
				(*this)[size() - 1]->color, Vec2(tScale));
			font.Render(" " + (*this)[size() - 1]->name + "  " + to_string((*this)[size() - 1].count) + "  " + (*this)[size() - 1]->typeName,
				Vec2(-ScrWidth() + scale * 2, -ScrHeight() + scale * 2 * width + 1), scale * 2, (*this)[size() - 1]->color);
		}
	}

	bool CanUse(Vec3 pos, Vec3 dir, float lastUse, float shootSpeed)
	{
		if (isBuilding)
		{
			if (tTime - lastUse < bItem.useTime * shootSpeed)
				return false;
			for (Tower* tower : Defences::Towers::towers)
				if (CanMake(tower->recipe))
					return true;
			return false;
		}
		int index = currentIndex + width * currentRow;
		return (*this)[index].count != 0 && tTime - lastUse >= (*this)[index]->useTime * shootSpeed;
	}

	bool Use(Vec3 pos, Vec3 dir) // True means stack is empty* and false means the opposite.
	{ // *if isBuilding is true then it will return true.
		if (isBuilding)
		{
			vector<Tower*> buildableTowers;
			for (Tower* tower : Defences::Towers::towers)
				if (CanMake(tower->recipe))
					buildableTowers.push_back(tower);
			if (buildableTowers.size() == 0) return true;
			if (buildableTowers[currentIndex]->TryCreate(this, pos, dir, player))
			{
				// If we created a tower then we should make sure that currentIndex is still a valid value.
				buildableTowers.clear();
				for (Tower* tower : Defences::Towers::towers)
					if (CanMake(tower->recipe))
						buildableTowers.push_back(tower);

				if (buildableTowers.size() == 0)
					currentIndex = 0;
				else
					currentIndex = currentIndex % buildableTowers.size();
			}
			return true;
		}
		int index = currentIndex + currentRow * width;
		ItemInstance& currentItem = (*this)[index];
		currentItem->Use(currentItem, pos, dir * currentItem->range, player, player->name, nullptr, 0);
		return RemoveIfEmpty(index);
	}

	/*TryUse TryUse(float lastUse, float shootSpeed, Vec3 dir)
	{
		if (isBuilding)
		{
			if (tTime - lastUse < bItem.useTime * shootSpeed)
				return false;
			vector<Tower*> buildableTowers;
			for (Tower* tower : Defences::Towers::towers)
				if (CanMake(tower->recipe))
					buildableTowers.push_back(tower);
					return true;
			return false;
			for (Tower* tower : Defences::Towers::towers)
				if (CanMake(tower->recipe))
			if (buildableTowers.size() == 0) return true;
			if (buildableTowers[currentIndex]->TryCreate(this, player->pos, dir, player))
			{
				// If we created a tower then we should make sure that currentIndex is still a valid value.
				buildableTowers.clear();
				for (Tower* tower : Defences::Towers::towers)
					if (CanMake(tower->recipe))
						buildableTowers.push_back(tower);

				if (buildableTowers.size() == 0)
					currentIndex = 0;
				else
					currentIndex = currentIndex % buildableTowers.size();
			}
			return true;
		}
		int index = currentIndex + width * currentRow;
		return (*this)[index].count != 0 && tTime - lastUse >= (*this)[index]->useTime * shootSpeed;
		int index = currentIndex + currentRow * width;
		ItemInstance& currentItem = (*this)[index];
		currentItem->Use(currentItem, player->pos, dir * currentItem->range, player, player->name, nullptr, 0);
		return RemoveIfEmpty(index);
	}*/

	bool CanUseOffhand(Vec3 pos, Vec3 dir, float lastUse, float shootSpeed)
	{
		if (isBuilding)
			return tTime - lastUse >= bItem.useTime * shootSpeed && game->entities->RaycastEnt(pos, dir, 30, MaskF::IsAlly, player).index != -1;
		int index = size() - 1;
		return (*this)[index].count != 0 && tTime - lastUse >= (*this)[index]->useTime * shootSpeed;
	}

	bool UseOffhand(Vec3 pos, Vec3 dir) // True means stack is empty* and false means the opposite.
	{ // *if isBuilding is true then it will return true.
		if (isBuilding)
		{
			RaycastHit hit = game->entities->RaycastEnt(pos, player->dir, 30, MaskF::IsAlly, player);
			if (hit.index != -1)
			{
				Entity* hitEntity = (*game->entities)[hit.index].get();
				if (hitEntity != reinterpret_cast<Entity*>(game->base))
					hitEntity->ApplyHit(100000, player);

			}
			return true;
		}
		int index = size() - 1;
		ItemInstance& currentItem = (*this)[index];
		currentItem->Use(currentItem, pos, dir * currentItem->range, player, player->name, nullptr, 0);
		return RemoveIfEmpty(index);
	}
};

#define NUM_START_ITEMS 3

int difficultySeedSpawnQuantity[] = { 7, 5, 3 };
int difficultySeedSelectQuantity[] = { 6, 5, 4 };

#pragma region Psuedo-Virtuals
enum class PMOVEMENT
{
	DEFAULT, JETPACK
};
vector<std::function<void(Player* player)>> pMovements;

enum class PRIMARY
{
	SLINGSHOT, ENG_SHOOT, CIRCLEGUN, CHOMP
};
vector< std::function<bool(Player* player)>> primaries;

enum class OFFHAND
{
	SLINGSHOT, ENG_SHOOT, CIRCLEGUN, CHOMP
};
vector< std::function<bool(Player* player)>> offhands;

enum class SECONDARY
{
	GRENADE_THROW, FLAME_PUDDLE, ENGMODEUSE, VINE_SHOT
};
vector< std::function<bool(Player* player)>> secondaries;

enum class UTILITY
{
	TACTICOOL_ROLL, FLAMETHROWER, ENGMODESWAP
};
vector< std::function<bool(Player* player)>> utilities;
#pragma endregion
#pragma region Player Types
EntityData playerData = EntityData(UPDATE::PLAYER, VUPDATE::FRICTION, DUPDATE::PLAYER, UIUPDATE::PLAYER, ONDEATH::PLAYER);
class Player : public LightBlock
{
public:
#pragma region Variables
	Base* base = nullptr;
	Entity* heldEntity = nullptr, *currentMenuedEntity = nullptr;
	PlayerInventory items;
	Items startItems;
	bool startSeeds[UnEnum(SEEDINDICES::COUNT)] = { false };
	vector<SEEDINDICES> blacklistedSeeds;
	Vec2 placingDir = north;
	float moveSpeed, maxSpeed, vacDist, vacSpeed, maxVacSpeed, holdMoveSpeed, maxHoldMoveSpeed, holdMoveWeight, shootSpeed,
		lastPrimary = 0, primaryTime, lastOffhand = 0, offhandTime, lastSecondary = 0, secondaryTime, lastUtility = 0, utilityTime, lastJump = 0;
	bool vacBoth, vacCollectibles, shouldVacuum = true, shouldPickup = true, shouldScroll = true, invOpen = false,
		shouldRenderInventory = true;
	float timeSinceHit = 0, timeTillHeal = 0;
	PMOVEMENT movement;
	PRIMARY primary;
	OFFHAND offhand;
	SECONDARY secondary;
	UTILITY utility;
	vector<TimedEvent> events{};
	float iTime = 0, sTime = 0; // Invincibility time, if <= 0 takes damage, else doesn't.
	Inputs inputs;
	float yaw = 0, pitch = 0;
	Vec3 camDir = vZero; // The direction that the camera is facing.
	Vec3 moveDir = vZero; // camDir but without the verticality.
	Vec3 rightDir = vZero; // moveDir but rotated 90 degrees to the right.
	Vec3 upDir = vZero; // Perpendicular to rightDir and moveDir.
#pragma endregion

	Player(EntityData* data, bool vacBoth = false, bool vacCollectibles = true, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8,
		float holdMoveSpeed = 32, float maxHoldMoveSpeed = 8, float holdMoveWeight =  4, float vacDist = 6, float vacSpeed = 16,
		float maxVacSpeed = 16, float shootSpeed = 1, float primaryTime = 1, float offhandTime = 1, float secondaryTime = 1,
		float utilityTime = 1, PMOVEMENT movement = PMOVEMENT::DEFAULT, PRIMARY primary = PRIMARY::SLINGSHOT, OFFHAND offhand = OFFHAND::SLINGSHOT,
		SECONDARY secondary = SECONDARY::GRENADE_THROW, UTILITY utility = UTILITY::TACTICOOL_ROLL,
		RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10,
		float mass = 1, float bounciness = 0,
		int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}, vector<SEEDINDICES> blacklistedSeeds = {}) :
		LightBlock(data, lightColor, lightOrDark, range, vZero, radius, color, color2, mass, bounciness, maxHealth, health, name, PLAYER_A), vacDist(vacDist),
		moveSpeed(moveSpeed), holdMoveSpeed(holdMoveSpeed), startItems(startItems), blacklistedSeeds(blacklistedSeeds), vacBoth(vacBoth),
		vacCollectibles(vacCollectibles), vacSpeed(vacSpeed), maxSpeed(maxSpeed), maxVacSpeed(maxVacSpeed), shootSpeed(shootSpeed),
		primaryTime(primaryTime), offhandTime(offhandTime), secondaryTime(secondaryTime), utilityTime(utilityTime), movement(movement), primary(primary), offhand(offhand),
		secondary(secondary), utility(utility), maxHoldMoveSpeed(maxHoldMoveSpeed), holdMoveWeight(holdMoveWeight)
	{
		uiActive = true;
	}

	void Start() override
	{
		LightBlock::Start(); // Set up all of the light stuff.

		iTime = 0;
		sTime = 0;

		lastPrimary = tTime - primaryTime;
		lastSecondary = tTime - secondaryTime;
		lastUtility = tTime - utilityTime;
		shouldVacuum = true;

		items = PlayerInventory(this, 10, 4, startItems);

		for (int i = 0; i < UnEnum(SEEDINDICES::COUNT); i++)
			if (startSeeds[i])
				items.push_back(Resources::Seeds::plantSeeds[i]->Clone(difficultySeedSpawnQuantity[game->settings.difficulty]));
		items.currentIndex = 0; // Set active ammo type to the first ammo type in inventory.
	}

	Player(Player* baseClass, bool* startSeeds, Vec3 pos) :
		Player(*baseClass)
	{
		std::copy(startSeeds, startSeeds + sizeof(bool) * UnEnum(SEEDINDICES::COUNT), this->startSeeds);
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	virtual unique_ptr<Player> PClone(bool* startSeeds, Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr)
	{
		return make_unique<Player>(this, startSeeds, pos);
	}

	int ApplyHit(int damage, Entity* damageDealer) override
	{
		if (damageDealer == this)
			return LightBlock::ApplyHit(damage, damageDealer);
		if (iTime <= 0)
		{
			if (damage > 0)
			{
				iTime++;
				timeSinceHit = 0;
				game->screenShake += damage * 0.05f;
			}
			return LightBlock::ApplyHit(damage, damageDealer);
		}
		return HitResult::LIVED;
	}

	void UnAttach(Entity* entity) override
	{
		if (heldEntity == entity)
			heldEntity = nullptr;
	}

	void MoveAttachment(Entity* from, Entity* to) override
	{
		if (heldEntity == from)
			heldEntity = to;
	}

	virtual bool Grounded()
	{
		return game->entities->OverlapsAny(pos + Vec3(0, 0, -radius), radius - 0.01f, MaskF::IsCorporeal, this);
		//return game->entities->OverlapsTile(pos + Vec3(0, 0, -radius), radius - 0.01f);
	}
};

EntityData flareData = EntityData(UPDATE::FLARE, VUPDATE::FRICTION, DUPDATE::PLAYER, UIUPDATE::FLARE, ONDEATH::PLAYER);
class Flare : public Player
{
public:
	float currentFlame, maxFlame;

	Flare(EntityData* data, float maxFlame, bool vacBoth = false, bool vacCollectibles = true, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8,
		float holdMoveSpeed = 32, float maxHoldMoveSpeed = 8, float holdMoveWeight = 4, float vacDist = 6, float vacSpeed = 16,
		float maxVacSpeed = 16, float shootSpeed = 1, float primaryTime = 1, float offhandTime = 1, float secondaryTime = 1,
		float utilityTime = 1, PMOVEMENT movement = PMOVEMENT::DEFAULT, PRIMARY primary = PRIMARY::SLINGSHOT, OFFHAND offhand = OFFHAND::SLINGSHOT,
		SECONDARY secondary = SECONDARY::GRENADE_THROW, UTILITY utility = UTILITY::TACTICOOL_ROLL,
		RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10,
		float mass = 1, float bounciness = 0,
		int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}, vector<SEEDINDICES> blacklistedSeeds = {}) :
		Player(data, vacBoth, vacCollectibles, radius, moveSpeed, maxSpeed, holdMoveSpeed, maxHoldMoveSpeed, holdMoveWeight, vacDist, vacSpeed,
			maxVacSpeed, shootSpeed, primaryTime, offhandTime, secondaryTime, utilityTime, movement, primary, offhand, secondary, utility, color, color2,
			lightColor, lightOrDark, range, mass, bounciness, maxHealth, health, name, startItems, blacklistedSeeds),
		currentFlame(maxFlame), maxFlame(maxFlame)
	{ }

	Flare(Flare* baseClass, bool* startSeeds, Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) :
		Flare(*baseClass)
	{
		this->baseClass = baseClass;
		std::copy(startSeeds, startSeeds + sizeof(bool) * UnEnum(SEEDINDICES::COUNT), this->startSeeds);
		this->pos = pos;
		this->creator = creator;
		Start();
	}

	unique_ptr<Player> PClone(bool* startSeeds, Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr)
	{
		return make_unique<Flare>(this, startSeeds, pos, dir, creator);
	}
};

enum class ENGMODE
{
	ROVER, DRONE, REMOVE_DRONE, COUNT
};
string engModeStr[] = {"Rover", "Drone", "Remove Drone"};

class Drone;
EntityData engineerData = EntityData(UPDATE::ENGINEER, VUPDATE::FRICTION, DUPDATE::PLAYER, UIUPDATE::ENGINEER, ONDEATH::PLAYER);
class Engineer : public Player
{
public:
	ENGMODE engMode = ENGMODE::ROVER;
	vector<SpringFadeCircle*> drones;
	float currentJetpackFuel = 0, jetpackFuel, jetpackForce;

	Engineer(EntityData* data, float jetpackFuel, float jetpackForce, bool vacBoth = false, bool vacCollectibles = true, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8,
		float holdMoveSpeed = 32, float maxHoldMoveSpeed = 8, float holdMoveWeight = 4, float vacDist = 6, float vacSpeed = 16,
		float maxVacSpeed = 16, float shootSpeed = 1, float primaryTime = 1, float offhandTime = 1, float secondaryTime = 1,
		float utilityTime = 1, PMOVEMENT movement = PMOVEMENT::DEFAULT, PRIMARY primary = PRIMARY::SLINGSHOT, OFFHAND offhand = OFFHAND::SLINGSHOT,
		SECONDARY secondary = SECONDARY::GRENADE_THROW, UTILITY utility = UTILITY::TACTICOOL_ROLL,
		RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10,
		float mass = 1, float bounciness = 0,
		int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}, vector<SEEDINDICES> blacklistedSeeds = {}) :
		Player(data, vacBoth, vacCollectibles, radius, moveSpeed, maxSpeed, holdMoveSpeed, maxHoldMoveSpeed, holdMoveWeight, vacDist, vacSpeed,
			maxVacSpeed, shootSpeed, primaryTime, offhandTime, secondaryTime, utilityTime, movement, primary, offhand, secondary, utility, color, color2,
			lightColor, lightOrDark, range, mass, bounciness, maxHealth, health, name, startItems, blacklistedSeeds),
		jetpackFuel(jetpackFuel), jetpackForce(jetpackForce)
	{ }

	Engineer(Engineer* baseClass, bool* startSeeds, Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) :
		Engineer(*baseClass)
	{
		this->baseClass = baseClass;
		std::copy(startSeeds, startSeeds + sizeof(bool) * UnEnum(SEEDINDICES::COUNT), this->startSeeds);
		this->pos = pos;
		this->creator = creator;
		engMode = ENGMODE::ROVER;
		drones = {};
		Start();
	}

	unique_ptr<Player> PClone(bool* startSeeds, Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr)
	{
		return make_unique<Engineer>(this, startSeeds, pos, dir, creator);
	}
};
#pragma endregion
#pragma region Player Instances
Player soldier = Player(&playerData, false, true, 0.4f, 32, 8, 32, 8, 4, 6, 256, 32, 1, 0, 0, 2, 4, PMOVEMENT::DEFAULT,
	PRIMARY::SLINGSHOT, OFFHAND::SLINGSHOT, SECONDARY::GRENADE_THROW, UTILITY::TACTICOOL_ROLL, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), true, 20, 5, 0.25f, 100, 50,
	"Soldier", Items({ Resources::copper.Clone(10) }),
	vector<SEEDINDICES>({ SEEDINDICES::COPPER, SEEDINDICES::SHADE, SEEDINDICES::CHEESE, SEEDINDICES::QUARTZ_S }));

Flare flare = Flare(&flareData, 100, false, true, 0.4f, 32, 8, 32, 8, 2, 4, 256, 32, 1, 0, 0, 2, 0.0625f, PMOVEMENT::DEFAULT,
	PRIMARY::SLINGSHOT, OFFHAND::SLINGSHOT, SECONDARY::FLAME_PUDDLE, UTILITY::FLAMETHROWER, RGBA(255, 255), RGBA(0, 0, 255), JRGB(127, 127, 127), true, 5.f, 1.5f,
	0.25f, 100, 50, "Flare", Items({ Resources::rock.Clone(10) }),
	vector<SEEDINDICES>({ SEEDINDICES::COAL, SEEDINDICES::ROCK, SEEDINDICES::SHADE, SEEDINDICES::CHEESE, SEEDINDICES::QUARTZ_V }));

Engineer engineer = Engineer(&engineerData, 2, 3, false, true, 0.4f, 32, 8, 32, 8, 2, 4, 0, 0, 1, 0, 0, 2, 0, PMOVEMENT::JETPACK,
	PRIMARY::ENG_SHOOT, OFFHAND::ENG_SHOOT, SECONDARY::ENGMODEUSE, UTILITY::ENGMODESWAP, RGBA(255, 0, 255), RGBA(0, 0, 0), JRGB(127, 127, 127), true, 20, 5,
	0.25f, 100, 50, "Engineer", Items({ Resources::silver.Clone(10) }),
	vector<SEEDINDICES>({ SEEDINDICES::SILVER, SEEDINDICES::SHADE, SEEDINDICES::CHEESE, SEEDINDICES::QUARTZ_S }));

vector<Player*> characters = { &soldier, &flare, &engineer };
#pragma endregion
#pragma region Player Creations
EntityData grenadeData = EntityData(UPDATE::GRENADE, VUPDATE::FRICTION, DUPDATE::DTOCOL, UIUPDATE::ENTITY, ONDEATH::LIGHTBLOCK);
class Grenade : public LightBlock
{
public:
	int damage;
	float pushForce, explosionRadius, startTime, timeTill, baseRange;

	Grenade(EntityData* data, int damage, float pushForce, float explosionRadius, float timeTill, JRGB lightColor, float range, float radius, RGBA color, float mass, float bounciness, int health, string name) :
		LightBlock(data, lightColor, true, range, vZero, radius, color, RGBA(), mass, bounciness, health, health, name),
		damage(damage), explosionRadius(explosionRadius), startTime(0), timeTill(timeTill), baseRange(range), pushForce(pushForce)
	{ }

	Grenade(Grenade* baseClass, Vec3 pos, Vec3 dir, Entity* creator) :
		Grenade(*baseClass)
	{
		this->baseClass = baseClass;
		this->pos = pos;
		this->dir = dir;
		vel = dir;
		if (creator != nullptr)
		{
			allegiance = creator->allegiance;
			this->creator = creator;
		}
		startTime = tTime;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr) override
	{
		return make_unique<Grenade>(this, pos, dir, creator);
	}
};

Grenade grenade = Grenade(&grenadeData, 60, 100, 5, 3, JRGB(255, 255), 15, 0.25f, RGBA(255, 255), 0.25f, 0, 60, "Grenade");

EntityData flamePuddleData = EntityData(UPDATE::FLAME_PUDDLE, VUPDATE::ENTITY, DUPDATE::FADEOUTGLOW, UIUPDATE::ENTITY, ONDEATH::FADEOUTGLOW);
class FlamePuddle : public FadeOutGlow
{
public:
	int damage;
	float timePer;

	FlamePuddle(EntityData* data, int damage, float timePer, float range, float totalFadeTime = 1.0f, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA()) :
		FadeOutGlow(data, range, totalFadeTime, pos, radius, color), damage(damage), timePer(timePer)
	{ }

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		unique_ptr<FlamePuddle> puddle = make_unique<FlamePuddle>(data, damage, timePer, startRange, totalFadeTime, pos, radius, color);
		puddle->creator = creator;
		puddle->allegiance = creator->allegiance;
		return std::move(puddle);
	}
};

FlamePuddle flamePuddle = FlamePuddle(&flamePuddleData, 10, 0.1f, 5, 2, vZero, 1.5f, RGBA(156, 106, 78, 127));

EntityData flameGlobData = EntityData(UPDATE::FLAME_GLOB, VUPDATE::FRICTION, DUPDATE::ENTITY, UIUPDATE::ENTITY, ONDEATH::LIGHTBLOCK);
class FlameGlob : public LightBlock
{
public:
	Entity* flamePuddle;

	FlameGlob(EntityData* data, Entity* flamePuddle, JRGB lightColor, float range, float radius, RGBA color, float mass, float bounciness, int health, string name) :
		LightBlock(data, lightColor, true, range, vZero, radius, color, RGBA(), mass, bounciness, health, health, name), flamePuddle(flamePuddle)
	{
		corporeal = false;
	}

	FlameGlob(FlameGlob* baseClass, Vec3 pos, Vec3 dir, Entity* creator) :
		FlameGlob(*baseClass)
	{
		this->baseClass = baseClass;
		this->pos = pos;
		this->dir = dir;
		vel = dir;
		if (creator != nullptr)
		{
			allegiance = creator->allegiance;
			this->creator = creator;
		}
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir, Entity* creator) override
	{
		return make_unique<FlameGlob>(this, pos, dir, creator);
	}
};

FlameGlob flameGlob = FlameGlob(&flameGlobData, &flamePuddle, JRGB(255, 127), 15, 0.25f, RGBA(255, 255), 0.25f, 0, 60, "Grenade");

class FlareFlame : public Item
{
public:
	int flameRestore;

	FlareFlame(ITEMTYPE type, int flameRestore, string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), int damage = 1,
		float range = 15.0f, float useTime = 0.25f, float speed = 12, float radius = 0.4f, bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		Item(type, name, typeName, intType, color, damage, range, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health),
		flameRestore(flameRestore)
	{
		itemOD = ITEMOD::FLARE_FLAME;
	}
};

namespace ItemODs
{
	void FlareFlameOD(ItemInstance item, Vec2 pos, Vec2 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		if (callType != 0 && game->player && game->player->name == "Flare")
			static_cast<Flare*>(game->player)->currentFlame += static_cast<FlareFlame*>(item.Type())->flameRestore;
	}
}

FlareFlame flareFlame = FlareFlame(ITEMTYPE::FLARE_FLAME, 2, "Flare Flame", "Player Creation", 0, RGBA(255, 93, 0), 10, 15, 0.0625f, 18, 1);

EntityData turretData = EntityData(UPDATE::TURRET, VUPDATE::FRICTION, DUPDATE::TURRET, UIUPDATE::ENTITY, ONDEATH::LIGHTBLOCK);
class Turret : public LightBlock
{
public:
	float timeTill, timePer;
	PlayerInventory* items = nullptr;
	Engineer* leader = nullptr;
	
	Turret(EntityData* data, float timePer,
		JRGB lightColor, float range, float radius, RGBA color, RGBA color2, float mass, float bounciness, int maxHealth, int health, string name) :
		LightBlock(data, lightColor, true, range, vZero, radius, color, color2, mass, bounciness, maxHealth, health, name, PLAYER_A),
		timePer(timePer), timeTill(timePer)
	{ }

	Turret(Turret* baseClass, Vec3 pos, Vec3 dir, Engineer* creator) :
		Turret(*baseClass)
	{
		this->pos = pos;
		this->dir = dir;
		this->creator = creator;
		leader = creator;
		items = &creator->items;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Turret>(this, pos, dir, static_cast<Engineer*>(creator));
	}
};

Turret turret = Turret(&turretData, 2, JRGB(255), 10, 0.5f, RGBA(255), RGBA(), 0.5f, 0.f, 50, 50, "Turret");

EntityData roverData = EntityData(UPDATE::ROVER, VUPDATE::FRICTION, DUPDATE::ROVER, UIUPDATE::ENTITY, ONDEATH::LIGHTBLOCK);
class Rover : public LightBlock
{
public:
	float vacSpeed, maxVacSpeed, vacDist, moveSpeed, maxSpeed, timeTillJump, timePerJump, remainingLifetime, lifetime;
	Items* items = nullptr;
	Engineer* leader = nullptr;

	Rover(EntityData* data, float vacSpeed, float maxVacSpeed, float vacDist, float moveSpeed, float maxSpeed, float timePerJump, float lifetime,
		JRGB lightColor, float range, float radius, RGBA color, RGBA color2, float mass, float bounciness, int maxHealth, int health, string name) :
		LightBlock(data, lightColor, true, range, vZero, radius, color, color2, mass, bounciness, maxHealth, health, name, PLAYER_A),
		vacSpeed(vacSpeed), maxVacSpeed(maxVacSpeed), vacDist(vacDist), moveSpeed(moveSpeed), maxSpeed(maxSpeed),
		timeTillJump(0), timePerJump(timePerJump), remainingLifetime(lifetime), lifetime(lifetime)
	{ }

	Rover(Rover* baseClass, Vec3 pos, Vec3 dir, Engineer* creator) :
		Rover(*baseClass)
	{
		this->pos = pos;
		this->dir = Normalized(dir);
		this->creator = creator;
		leader = creator;
		items = &creator->items;
		remainingLifetime = lifetime;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Rover>(this, pos, dir, static_cast<Engineer*>(creator));
	}
};

Rover rover = Rover(&roverData, 32, 8, 4, 4, 4, 2, 10, JRGB(255, 255), 3, 0.25f, RGBA(255, 255), RGBA(), 0.1f, 0, 20, 20, "Rover");
#pragma endregion
#pragma region Base Types
EntityData baseData = EntityData(UPDATE::BASE, VUPDATE::FRICTION, DUPDATE::DTOCOL, UIUPDATE::ENTITY, ONDEATH::BASE);
class Base : public LightBlock
{
public:
	Player* player = nullptr;
	float timeSinceHit = 0, timeTillHeal = 0;

	Base(EntityData* data, JRGB lightColor, bool lightOrDark, float range, float radius, RGBA color, RGBA color2, float mass,
		float bounciness, int health, string name) :
		LightBlock(data, lightColor, lightOrDark, range, vZero, radius, color, color2, mass, bounciness, health, health, name) { }

	Base(Base* baseClass, Vec3 pos) :
		Base(*baseClass)
	{
		this->pos = pos;
		player = game->player;
		allegiance = player->allegiance;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Base>(this, pos);
	}

	int ApplyHit(int damage, Entity* damageDealer) override
	{
		if (damage > 0)
			timeSinceHit = 0;
		return LightBlock::ApplyHit(damage, damageDealer);
	}
};
#pragma endregion
#pragma region Base Instances
Base soldierBase = Base(&baseData, JRGB(127, 127, 255), true, 15, 2.5f, RGBA(127, 127, 255), RGBA(), 10, 0.25f, 360, "base");
vector<Base*> charBases = { &soldierBase, &soldierBase, &soldierBase };
#pragma endregion

namespace OnDeaths
{
	void PlayerOD(Entity* entity, Entity* damageDealer)
	{
		Player* player = static_cast<Player*>(entity);

		game->cursorUnlockCount = 1;

		player->base->OnDeath(ONDEATH::LIGHTBLOCK, damageDealer);
		player->OnDeath(ONDEATH::LIGHTBLOCK, damageDealer);
		playerAlive = false;
		deathName = player->name;
		if (damageDealer != nullptr)
			deathCauseName = damageDealer->name;
		else
			deathCauseName = "The planet";
	}

	void BaseOD(Entity* entity, Entity* damageDealer)
	{
		Base* base = static_cast<Base*>(entity);

		game->cursorUnlockCount = 1;

		base->player->OnDeath(ONDEATH::LIGHTBLOCK, damageDealer);
		base->OnDeath(ONDEATH::LIGHTBLOCK, damageDealer);
		playerAlive = false;
		deathName = base->player->name + "'s base";
		if (damageDealer != nullptr)
			deathCauseName = damageDealer->name;
		else
			deathCauseName = "The planet";
	}
}

namespace ItemUs
{
	void WaveModifierU(ItemInstance item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		game->planet->faction1Spawns->superWave ^= true;
		game->entities->push_back(make_unique<FadeOutGlow>(&fadeOutGlowData, 8.f, 3.f, pos, 4.f, item->color));
		game->screenShake++;
	}
}

void PlayerInventory::Update(bool shouldScroll)
{
#pragma region Crafting
	if (!isBuilding)
	{
		if (game->inputs.keys[KeyCode::BUILD].pressed)
		{
			isBuilding = true;
			int buildCount = 0;
			for (int i = 0; i < Defences::Towers::towers.size(); i++)
				if (CanMake(Defences::Towers::towers[i]->recipe)) buildCount++;
			if (buildCount == 0)
				currentIndex = 0;
			else
				currentIndex = JMod(currentIndex + game->inputs.mouseScroll, buildCount);
			return;
		}
	}
	else if (game->inputs.keys[KeyCode::BUILD].pressed || game->inputs.keys[KeyCode::INVENTORY].pressed)
	{
		isBuilding = false;
	}
	else if (shouldScroll)
	{
		int buildCount = 0;
		for (int i = 0; i < Defences::Towers::towers.size(); i++)
			if (CanMake(Defences::Towers::towers[i]->recipe)) buildCount++;
		if (buildCount == 0)
			currentIndex = 0;
		else
			currentIndex = JMod(currentIndex + game->inputs.mouseScroll, buildCount);
		return;
	}
#pragma endregion

	isHovering = false;
	if (game->inputs.keys[KeyCode::INVENTORY].pressed)
	{
		currentSelected = -1;
		if (isOpen)
		{
			isOpen = false;
			game->cursorUnlockCount--;
		}
		else
		{
			isOpen = true;
			game->cursorUnlockCount++;
		}
	}
	if (shouldScroll)
		currentIndex = JMod(currentIndex + game->inputs.mouseScroll, int(width));

	currentRow = JMod(currentRow + int(game->inputs.keys[KeyCode::ROW_RIGHT].pressed) -
		int(game->inputs.keys[KeyCode::ROW_LEFT].pressed), int(height));

	int scrHeight = ScrHeight();
	float scale = scrHeight / (3.f * width), scale2 = scale * 0.2f;
	if (isOpen && game->inputs.screenMousePosition.x >= 0 && game->inputs.screenMousePosition.y >= 0 &&
		game->inputs.screenMousePosition.x < scale * height && game->inputs.screenMousePosition.y < scale * width)
	{
		isHovering = true;
		if (game->inputs.keys[KeyCode::PRIMARY].pressed)
		{
			iVec2 rPos = game->inputs.screenMousePosition / scale;

			int newSelected = rPos.x * width + rPos.y;
			if (currentSelected == -1)
				currentSelected = newSelected;
			else if (currentSelected == newSelected)
				currentSelected = -1;
			else
			{
				ItemInstance temp = (*this)[newSelected];
				(*this)[newSelected] = (*this)[currentSelected];
				(*this)[currentSelected] = temp;
				currentSelected = -1;
			}
		}
		game->player->inputs.keys[KeyCode::PRIMARY] = KeyPress(false, false, true);
	}
	else if (isOpen && game->inputs.screenMousePosition.x >= scale * height && game->inputs.screenMousePosition.y >= 0 &&
		game->inputs.screenMousePosition.x < scale * (height + 1) && game->inputs.screenMousePosition.y < scale)
	{
		isHovering = true;
		if (game->inputs.keys[KeyCode::PRIMARY].pressed)
		{
			int newSelected = size() - 1;
			if (currentSelected == -1)
				currentSelected = newSelected;
			else if (currentSelected == newSelected)
				currentSelected = -1;
			else
			{
				ItemInstance temp = (*this)[newSelected];
				(*this)[newSelected] = (*this)[currentSelected];
				(*this)[currentSelected] = temp;
				currentSelected = -1;
			}
		}
		game->player->inputs.keys[KeyCode::PRIMARY] = KeyPress(false, false, true);
	}
	else if (game->inputs.keys[KeyCode::PRIMARY].pressed) currentSelected = -1;
}

Vec3 Inputs::MoveDir()
{
	return Normalized((float(keys[KeyCode::RIGHT].held) - float(keys[KeyCode::LEFT].held)) * game->player->rightDir +
		(float(keys[KeyCode::UP].held) - float(keys[KeyCode::DOWN].held)) * game->player->moveDir);
}

namespace Updates
{
	void PlayerU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);

		player->inputs = game->inputs;

		// A bit of input reading:
		player->shouldVacuum ^= player->inputs.keys[KeyCode::CROUCH].pressed;

		// Update items:
		player->items.Update(player->sTime <= 0 && player->shouldScroll);

		// Decrease iTime and sTime:
		player->iTime = max(0.f, player->iTime - game->dTime);
		player->sTime = max(0.f, player->sTime - game->dTime);

		// Healing:
		player->timeSinceHit += game->dTime;
		player->timeTillHeal -= game->dTime * min(1.f, 0.1f * player->timeSinceHit);
		if (player->timeTillHeal <= 0)
		{
			player->timeTillHeal++;
			player->ApplyHit(-1, player);
		}

		// Update player events:
		for (int i = 0; i < player->events.size(); i++)
			if (player->events[i].function(player, &player->events[i]))
				player->events.erase(player->events.begin() + i);

		/*
		if (player->currentMenuedEntity != nullptr && !player->currentMenuedEntity->Overlaps(player->inputs.mousePosition3 + player->pos, 0))
		{
			player->currentMenuedEntity->uiActive = false;
			player->currentMenuedEntity = nullptr;
		}

		Entity* hitEntity = nullptr;
		if (player->currentMenuedEntity == nullptr &&
			(hitEntity = game->entities->FirstOverlap(player->inputs.mousePosition3 + player->pos, 0, MaskF::IsCorporeal, player)) != nullptr)
		{
			if (player->currentMenuedEntity != nullptr)
				player->currentMenuedEntity->uiActive = false;
			player->currentMenuedEntity = hitEntity;
			player->currentMenuedEntity->uiActive = true;
		}*/

		Vec2 mouseOffset = game->inputs.mouseOffset;
		player->yaw -= mouseOffset.x;
		player->pitch = ClampF(player->pitch + mouseOffset.y, -89, 89);

		player->camDir.x = cos(glm::radians(player->yaw)) * cos(glm::radians(player->pitch));
		player->camDir.y = sin(glm::radians(player->yaw)) * cos(glm::radians(player->pitch));
		player->camDir.z = sin(glm::radians(player->pitch));
		player->camDir = glm::normalize(player->camDir);
		player->moveDir = glm::normalize(Vec3(Vec2(player->camDir), 0));
		player->rightDir = glm::normalize(glm::cross(player->camDir, up));
		player->upDir = glm::normalize(glm::cross(player->rightDir, player->camDir));
		player->dir = player->camDir;

		if (player->sTime <= 0 && game->uiMode == UIMODE::INGAME)
		{
			pMovements[UnEnum(player->movement)](player); // This handles all of the locomotion of the player.

			ItemInstance currentShootingItem = player->items.GetCurrentItem();

			/*if (player->inputs.middleMouse.released && player->heldEntity != nullptr)
			{
				player->heldEntity->observers.erase(std::find(player->heldEntity->observers.begin(), player->heldEntity->observers.end(), player));
				player->heldEntity = nullptr;
			}

			// Dragging code:
			if (player->heldEntity != nullptr)
				player->heldEntity->vel = TryAdd2(player->heldEntity->vel, Normalized(player->pos + player->inputs.mousePosition3 - player->heldEntity->pos) *
					game->dTime * player->holdMoveSpeed, player->maxHoldMoveSpeed * player->holdMoveWeight / max(player->holdMoveWeight, player->heldEntity->mass));

			// If can and should grab then do grag.
			if (player->heldEntity == nullptr && player->inputs.middleMouse.pressed &&
				(hitEntity = game->entities->FirstOverlap(player->inputs.mousePosition3 + player->pos, 0, MaskF::IsCorporeal, player)) != nullptr)
			{
				player->heldEntity = hitEntity;
				player->heldEntity->observers.push_back(player);
			}
			else*/ // Do primaries, secondaries, and/or utilities.
			{
				if (!player->items.isOpen && player->inputs.keys[KeyCode::PRIMARY].held && tTime - player->lastPrimary >= player->primaryTime &&primaries[UnEnum(player->primary)](player))
					player->lastPrimary = tTime;
				if (!player->items.isOpen && player->inputs.keys[KeyCode::OFFHAND].held && tTime - player->lastOffhand >= player->offhandTime && offhands[UnEnum(player->offhand)](player))
					player->lastOffhand = tTime;
				if (player->inputs.keys[KeyCode::SECONDARY].held && tTime - player->lastSecondary >= player->secondaryTime && secondaries[UnEnum(player->secondary)](player))
					player->lastSecondary = tTime;
				if (player->inputs.keys[KeyCode::UTILITY].held && tTime - player->lastUtility >= player->utilityTime && utilities[UnEnum(player->utility)](player))
					player->lastUtility = tTime;
			}

			 if (player->shouldVacuum)
				 game->entities->Vacuum(player->pos, player->vacDist, player->vacSpeed, player->maxVacSpeed, player->vacBoth, player->vacCollectibles);
		}

		if (player->shouldPickup)
		{
			vector<Entity*> collectibles = EntitiesOverlaps(player->pos, player->radius, game->entities->collectibles);
			for (Entity* collectible : collectibles)
			{
				player->items.push_back(((Collectible*)collectible)->baseItem);
				collectible->DestroySelf(player);
			}
		}
		if (player->shouldScroll)
			game->inputs.mouseScroll = 0;
	}

	void FlareU(Entity* entity)
	{
		Flare* flare = static_cast<Flare*>(entity);
		flare->currentFlame = min(flare->maxFlame, flare->currentFlame - game->dTime * 0.5f);
		float tempSpeed = flare->maxSpeed;
		flare->maxSpeed *= flare->currentFlame / flare->maxFlame;
		flare->maxSpeed = Lerp(flare->maxSpeed, tempSpeed, 0.5f);
		flare->Update(UPDATE::PLAYER);
		flare->maxSpeed = tempSpeed;
	}

	void EngineerU(Entity* entity)
	{
		Engineer* engineer = static_cast<Engineer*>(entity);
		engineer->shouldScroll = !engineer->inputs.keys[KeyCode::UTILITY].held;
		engineer->shouldPickup = false;
		engineer->Update(UPDATE::PLAYER);

		float offset = tTime * PI_F * 2 / 5;
		for (int i = 0; i < engineer->drones.size(); i++)
		{
			engineer->drones[i]->startTime = tTime; // Make sure that they never die.
			engineer->drones[i]->desiredPos = engineer->pos +
				Vec3(engineer->radius * 2 * CircPoint2(offset + PI_F * 2 * i / engineer->drones.size()), 0.f);
		}
	}

	void GrenadeU(Entity* entity)
	{
		Grenade* grenade = static_cast<Grenade*>(entity);
		if (tTime - grenade->startTime > grenade->timeTill)
		{
			CreateUpExplosion(grenade->pos, grenade->explosionRadius, grenade->color, grenade->name, 0, grenade->damage, grenade->creator);
			game->entities->VacuumBurst(grenade->pos, grenade->explosionRadius, -grenade->pushForce, 50.f, true, false);
			grenade->DestroySelf(nullptr);
			return;
		}
		grenade->range = grenade->baseRange * (0.5f - 0.5f * cosf(6 * PI_F * (tTime - grenade->startTime) / grenade->timeTill));
		grenade->lightSource->range = grenade->range;
	}

	void FlameGlobU(Entity* entity)
	{
		if (game->entities->OverlapsAny(entity->pos, entity->radius, MaskF::IsCorporealNotCreator, entity))
		{
			game->entities->push_back(static_cast<FlameGlob*>(entity)->flamePuddle->Clone(entity->pos, vZero, entity->creator));
			entity->DestroySelf(entity);
		}
	}

	void FlamePuddleU(Entity* entity)
	{
		FlamePuddle* puddle = static_cast<FlamePuddle*>(entity);
		if (int((tTime - puddle->startTime) / puddle->timePer) != int((tTime - puddle->startTime - game->dTime) / puddle->timePer))
		{
			vector<Entity*> hitEntities = game->entities->FindOverlaps(puddle->pos, puddle->radius, MaskF::IsNonAllyOrCreator, puddle);
			Flare* creator = static_cast<Flare*>(puddle->creator);
			for (Entity* e : hitEntities)
			{
				vector<StatusEffect>::iterator itr;
				if (e == creator && (itr = std::find_if(e->statuses.begin(), e->statuses.end(), [](StatusEffect const& s) { return s.status == STATUS::FLARE_FIRE; })) != e->statuses.end())
					itr->timeTillRemove = 3;
				else
					e->statuses.push_back(StatusEffect(e, STATUS::FLARE_FIRE, e == creator ? 3 : 1));

				creator->currentFlame++;
			}
		}
		puddle->Update(UPDATE::FADEOUT);
	}

	void TurretU(Entity* entity)
	{
		Turret* turret = static_cast<Turret*>(entity);
		turret->timeTill -= game->dTime;
		ItemInstance item = turret->items->GetCurrentItem();
		Entity* hitEntity = nullptr;
		if (turret->leader->shouldVacuum && item.type != ITEMTYPE::DITEM && item.count > 0 && turret->timeTill <= 0 &&
			(hitEntity = game->entities->FirstOverlap(turret->pos, item->range, MaskF::IsNonAlly, turret)) != nullptr)
		{
			turret->dir = hitEntity->pos - turret->pos;
			item->Use((*turret->items)[turret->items->currentIndex], turret->pos, turret->dir * item->range, turret, turret->name, hitEntity, 0);
			turret->items->RemoveIfEmpty(turret->items->currentIndex);
			turret->timeTill = turret->timePer * item->useTime;
		}
	}

	void RoverU(Entity* entity)
	{
		Rover* rover = static_cast<Rover*>(entity);

		rover->vel = Vec3(TryAdd2V2(rover->vel, rover->dir * game->dTime * (rover->moveSpeed + game->planet->friction),
			rover->maxSpeed), rover->vel.z);
		
		rover->remainingLifetime -= game->dTime;
		if (rover->remainingLifetime < 0)
			rover->DestroySelf(rover);
		rover->timeTillJump -= game->dTime;
		if (rover->timeTillJump < 0 &&
			game->entities->OverlapsTile(rover->pos + Vec3(0, 0, rover->radius * 2), rover->radius * 1.9f))
		{
			rover->vel.z += 7.f;
			rover->timeTillJump = rover->timePerJump;
		}
		game->entities->Vacuum(rover->pos, rover->vacDist, rover->vacSpeed, rover->maxVacSpeed, false, true);

		vector<Entity*> collectibles = EntitiesOverlaps(rover->pos, rover->radius, game->entities->collectibles);
		for (Entity* collectible : collectibles)
		{
			rover->leader->items.push_back(((Collectible*)collectible)->baseItem);
			collectible->DestroySelf(rover);
		}
	}

	void BaseU(Entity* entity)
	{
		Base* base = static_cast<Base*>(entity);

		// Healing:
		base->timeSinceHit += game->dTime;
		base->timeTillHeal -= game->dTime * min(1.f, 0.1f * base->timeSinceHit);
		if (base->timeTillHeal <= 0)
		{
			base->timeTillHeal++;
			base->ApplyHit(-1, base);
		}
	}
}

namespace DUpdates
{
	void PlayerDU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);
		if (player->items.isBuilding)
		{
			RaycastHit test = game->entities->RaycastEnt(entity->pos, player->camDir, 30, MaskF::IsAlly, entity);
			if (test.index != -1)
			{
				Entity* hitEntity = (*game->entities)[test.index].get();
				game->DrawCircle(hitEntity->pos, RGBA(255, 0, 0, 127), hitEntity->radius + 0.1f);
				game->DrawCircle(test.pos + test.norm * 0.1f, RGBA(195, 255, 127), 0.1f);
			}
		}
	}

	void TurretDU(Entity* entity)
	{
		Turret* turret = static_cast<Turret*>(entity);

		turret->DUpdate(DUPDATE::DTOCOL);
	}

	void RoverDU(Entity* entity)
	{
		Rover* rover = static_cast<Rover*>(entity);

		rover->DUpdate(DUPDATE::DTOCOL);
	}
}

namespace UIUpdates
{
	void PlayerUIU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);
		if (!game->showUI) return;

		// Item UI:
		if (player->shouldRenderInventory)
			player->items.DUpdate();


		// Ability UI:
		int scrHeight = ScrHeight();
		float scale = scrHeight / 30.f;
		iVec2 offset = iVec2(ScrWidth(), -scrHeight);
		offset.x -= scale * 2;
		game->DrawFBL(offset, RGBA(0, 0, 0), Vec2(scale));
		game->DrawFBL(offset, RGBA(255, 0, 0), Vec2(scale, scale * min(1.f, (tTime - player->lastUtility) / player->utilityTime)));
		offset.x -= scale * 2;
		game->DrawFBL(offset, RGBA(0, 0, 0), Vec2(scale));
		game->DrawFBL(offset, RGBA(255, 255, 0), Vec2(scale, scale * min(1.f, (tTime - player->lastSecondary) / player->secondaryTime)));
		offset.x -= scale * 2;
		game->DrawFBL(offset, RGBA(0, 0, 0), Vec2(scale));
		game->DrawFBL(offset, RGBA(0, 255, 0), Vec2(scale, scale * min(1.f, (tTime - player->lastPrimary) / (player->items.GetCurrentItem()->useTime * player->shootSpeed))));
		offset.x -= scale * 2;
		game->DrawFBL(offset, RGBA(0, 0, 0), Vec2(scale));
		game->DrawFBL(offset, RGBA(0, 0, 255), Vec2(scale, scale * min(1.f, (tTime - player->lastOffhand) / (player->items.GetCurrentOffhand()->useTime * player->shootSpeed))));

		// Reticle:
		game->DrawTextured(reticleSprite, 0, vZero, -Vec2(1.f / 24),
			RGBA(255, 255, 255, 127), Vec2(1.f / 12));
	}

	void FlareUIU(Entity* entity)
	{
		Flare* flare = static_cast<Flare*>(entity);
		if (!game->showUI) return;
		flare->UIUpdate(UIUPDATE::PLAYER);
		float scale = ScrHeight() / 30.f;
		iVec2 offset = iVec2(ScrWidth() - scale * 8, scale * 2 - ScrHeight());
		game->DrawFBL(offset, RGBA(18, 156, 163), Vec2(scale * 4, scale));
		game->DrawFBL(offset, RGBA(163, 71, 18), Vec2(4 * scale * flare->currentFlame / flare->maxFlame, scale));
		vector<StatusEffect>::iterator itr;
		if ((itr = std::find_if(flare->statuses.begin(), flare->statuses.end(), [](StatusEffect const& s) { return s.status == STATUS::FLARE_FIRE; })) != flare->statuses.end())
		{
			offset.y += scale * 2;
			float ratio = 0.33333f * itr->timeTillRemove;
			game->DrawFBL(offset, RGBA(41, 45, 74, ratio * 255), Vec2(scale * 4, scale));
			game->DrawFBL(offset, RGBA(15, 35, 166).CLerp(RGBA(104, 201, 212), 0.5f + 0.5f * sinf(tTime * PI_F * 3)).CLerp(
				RGBA(15, 35, 166).CLerp(RGBA(104, 201, 212), 0.5f), 1 - ratio), Vec2(4 * scale * ratio, scale));
		}
	}

	void EngineerUIU(Entity* entity)
	{
		Engineer* engineer = static_cast<Engineer*>(entity);
		if (!game->showUI) return;
		engineer->shouldRenderInventory = !engineer->inputs.keys[KeyCode::UTILITY].held;
		engineer->UIUpdate(UIUPDATE::PLAYER);
		if (engineer->inputs.keys[KeyCode::UTILITY].held)
		{
			float scale = ScrHeight() / (3.0f * max(8, int(UnEnum(ENGMODE::COUNT)))), scale2 = scale / 5.0f;
			for (int i = 0; i < UnEnum(ENGMODE::COUNT); i++)
			{
				font.Render(engModeStr[i],
					iVec2(static_cast<int>(-ScrWidth() + scale * 2), static_cast<int>(-ScrHeight() + scale * 2 * i)), scale * 2,
					i == UnEnum(engineer->engMode) ? RGBA(127, 127, 127) : RGBA(255, 255, 255));
			}
		}
	}
}

namespace PMovements
{
	void Default(Player* player)
	{
		player->vel = Vec3(TryAdd2V2(player->vel, Vec2(player->inputs.MoveDir()) * game->dTime * (player->moveSpeed + game->planet->friction),
			player->maxSpeed), player->vel.z);
		if (player->inputs.keys[KeyCode::JUMP].pressed && tTime - player->lastJump > 0.25f &&
			player->Grounded())
		{
			player->vel.z += 7 - min(0.f, player->vel.z);
			player->lastJump = tTime;
		}
	}

	void Jetpack(Player* player)
	{
		pMovements[UnEnum(PMOVEMENT::DEFAULT)](player);
		Engineer* engineer = static_cast<Engineer*>(player);
		if (engineer->Grounded())
		{
			engineer->currentJetpackFuel = engineer->jetpackFuel;
		}
		if (player->inputs.keys[KeyCode::JUMP].held && engineer->currentJetpackFuel > 0)
		{
			player->vel.z += (game->planet->gravity + engineer->jetpackForce) * game->dTime;
			engineer->currentJetpackFuel -= game->dTime;
		}
	}
}

namespace Primaries
{
	bool Slingshot(Player* player)
	{
		if (player->items.CanUse(player->pos, player->camDir, player->lastPrimary, player->shootSpeed))
		{
			player->lastPrimary = tTime + player->primaryTime;
			player->items.Use(player->pos, player->camDir);
		}
		return false;
	}

	bool EngShoot(Player* player)
	{
		if (!player->items.CanUse(player->pos, player->camDir, player->lastPrimary, player->shootSpeed))
			return false;

		player->lastPrimary = tTime + player->primaryTime;
		if (player->items.Use(player->pos, player->camDir)) return false;
		Engineer* engineer = static_cast<Engineer*>(player);
		for (SpringCircle* circle : engineer->drones)
			if (player->items.Use(circle->pos, player->camDir)) return false;
		return false;
	}
}

namespace Offhands
{
	bool Slingshot(Player* player)
	{
		if (player->items.CanUseOffhand(player->pos, player->camDir, player->lastOffhand, player->shootSpeed))
		{
			player->lastOffhand = tTime + player->offhandTime;
			player->items.UseOffhand(player->pos, player->camDir);
		}
		return false;
	}

	bool EngShoot(Player* player)
	{
		if (!player->items.CanUseOffhand(player->pos, player->camDir, player->lastOffhand, player->shootSpeed))
			return false;

		player->lastOffhand = tTime + player->offhandTime;
		if (player->items.UseOffhand(player->pos, player->camDir)) return false;
		Engineer* engineer = static_cast<Engineer*>(player);
		for (SpringCircle* circle : engineer->drones)
			if (player->items.UseOffhand(circle->pos, player->camDir)) return false;
		return false;
	}
}

namespace Secondaries
{
	bool GrenadeThrow(Player* player)
	{
#define GRENADE_THROW_SPEED 20.f
		game->entities->push_back(grenade.Clone(player->pos + player->camDir * player->radius, player->camDir * GRENADE_THROW_SPEED, player));
		return true;
	}

	bool ThrowFlame(Player* player)
	{
		game->entities->push_back(flameGlob.Clone(player->pos + player->camDir * player->radius, player->camDir * GRENADE_THROW_SPEED, player));
		return true;
	}

	bool EngModeUse(Player* player)
	{
		Engineer* engineer = static_cast<Engineer*>(player);
		switch (engineer->engMode)
		{
		case ENGMODE::ROVER:
		{
			game->entities->push_back(rover.Clone(engineer->pos + engineer->camDir * (engineer->radius + rover.radius), engineer->camDir, engineer));
			break;
		}
		case ENGMODE::DRONE:
		{
			unique_ptr<SpringFadeCircle> newDrone = make_unique<SpringFadeCircle>(100.f, 5.0f, 0.1f, engineer->pos, vZero, RGBA(0, 255), 1.f);
			engineer->drones.push_back(newDrone.get());
			game->entities->particles.push_back(std::move(newDrone));
			break;
		}
		case ENGMODE::REMOVE_DRONE:
		{
			engineer->drones.pop_back();
			break;
		}
		}
		return true;
	}
}

namespace Utilities
{
	bool TacticoolRoll(Player* player)
	{
#define TACTICOOL_ROLL_SPEED 15.f
		Vec3 dir = game->inputs.MoveDir();

		if (dir == vZero)
			return false;
		player->vel = TryAdd2(player->vel, dir * TACTICOOL_ROLL_SPEED, 20.f);
		player->iTime++;
		return true;
	}

	bool FlameThrower(Player* player)
	{
		Flare* flare = static_cast<Flare*>(player);
		if (flare->currentFlame < 1) return false;
		flare->currentFlame--;
		ItemInstance temp = ItemInstance(ITEMTYPE::FLARE_FLAME, 999);
		flareFlame.Use(temp, flare->pos, flare->dir, flare, flare->name, nullptr, 0);
		return true;
	}

	bool EngModeSwap(Player* player)
	{
		Engineer* engineer = static_cast<Engineer*>(player);
		engineer->engMode = ENGMODE(JMod(UnEnum(engineer->engMode) + game->inputs.mouseScroll, UnEnum(ENGMODE::COUNT)));
		game->inputs.mouseScroll = 0;
		return false;
	}
}

namespace StatusFuncs
{
#pragma region Starts
	void NoStr(StatusEffect& status) { }

	void WetStr(StatusEffect& status)
	{
		status.entity->frictionMultiplier *= 0.1f;
	}

#define FLARE_FIRE_SPD 2
	void FlareFireStr(StatusEffect& status)
	{
		if (status.entity == game->player && game->player->name == "Flare")
		{
			game->player->moveSpeed *= FLARE_FIRE_SPD;
			game->player->maxSpeed *= FLARE_FIRE_SPD;
		}
	}
#pragma endregion
#pragma region Updates
	void NoUpd(StatusEffect& status) { }

	void FireUpd(StatusEffect& status)
	{
		status.entity->ApplyHit(2, status.entity);
	}

	void FlareFireUpd(StatusEffect& status)
	{
		if (status.entity != game->player && game->player->name == "Flare")
			status.entity->ApplyHit(2, status.entity);
	}

	void BleedUpd(StatusEffect& status)
	{
		status.entity->ApplyHit(4, status.entity);
	}

	void WetUpd(StatusEffect& status)
	{
		status.entity->ApplyHit(1, status.entity);
	}
#pragma endregion
#pragma region Endings
	void NoEnd(StatusEffect& status) { }

	void WetEnd(StatusEffect& status)
	{
		status.entity->frictionMultiplier *= 10.f;
	}

	void FlareFireEnd(StatusEffect& status)
	{
		if (status.entity == game->player && game->player->name == "Flare")
		{
			game->player->moveSpeed /= FLARE_FIRE_SPD;
			game->player->maxSpeed /= FLARE_FIRE_SPD;
			game->player->ApplyHit(10, nullptr);
		}
	}
#pragma endregion
}

Vec3 Renderer::PlayerPos()
{
	return ((Game*)this)->player->pos;
}

Vec3 Renderer::BasePos()
{
	return ((Game*)this)->base->pos;
}