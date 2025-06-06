#include "Livestock.h"

class UseableItem : public Item
{
public:
	UseableItem(ITEMTYPE type, ItemU useFunction, string name = "NULL NAME", string typeName = "NULL TYPE NAME", int intType = 0, RGBA color = RGBA(), float useTime = 0) :
		Item(type, name, typeName, VUpdates::EntityVU, intType, color, 0, 0, useTime)
	{
		itemU = useFunction;
	}
};

class PlayerInventory : public Items
{
private:
	bool isHovering = false;
public:
	int currentIndex2 = 0;
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
				Remove(i);
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
		for (int i = 0; i < 999999; i++)
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

	void Remove(int index)
	{
		oldPlacements.push_back({ index, (*this)[index].type });
		(*this)[index] = ItemInstance(ITEMTYPE::DITEM, 0);
	}

	bool RemoveIfEmpty(int index) // True means did remove, false means did not.
	{
		if ((*this)[index].count > 0)
			return false;
		Remove(index);
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

			int currentHovered = -1;

			if (isHovering)
			{
				iVec2 rPos = game->inputs.screenMousePosition / scale;
				game->DrawFBL(Vec2(rPos) * scale * 2.f - Vec2(ScrDim()), RGBA(), Vec2(scale));
				currentHovered = rPos.x * width + rPos.y;
			}

			game->DrawFBL(Vec2(offset.x + height * scale * 2, offset.y), RGBA(127, 127, 127, 127), Vec2(scale));
			if ((*this)[size() - 1].count != 0)
				game->DrawTextured(spriteSheet, (*this)[size() - 1]->intType, tOffset, Vec2(tScale * height, 0),
					(*this)[size() - 1]->color, Vec2(tScale));
			if (currentSelected != -1)
				game->DrawFBL(offset + ToIV2(Vec2(currentSelected / width, currentSelected % width) * (scale * 2)), (*this)[currentSelected]->color, Vec2(scale));
			for (uint y = 0, i = 0; y < height; y++)
				for (uint x = 0; x < width; x++, i++)
				{
					if ((*this)[i].count == 0) continue;
					
					game->DrawTextured(spriteSheet, (*this)[i]->intType, tOffset, Vec2(tScale * y, tScale * x),
						(*this)[i]->color, Vec2(tScale));
				}

			bool shouldDisplayCurrentlySelected = (currentSelected != -1 && (*this)[currentSelected].count != 0);
			if (shouldDisplayCurrentlySelected || (currentHovered != -1 && (*this)[currentHovered].count != 0))
			{
				int index = shouldDisplayCurrentlySelected ? currentSelected : currentHovered;
				font.Render((*this)[index]->name + "  " + to_string((*this)[index].count) + "  " + (*this)[index]->typeName,
					iVec2(game->inputs.screenMousePosition * 2.f) - ScrDim(), scale * 2, (*this)[index]->color);
			}
			if (shouldDisplayCurrentlySelected)
				game->DrawTextured(spriteSheet, (*this)[currentSelected]->intType, tOffset,
					2.f * game->inputs.screenMousePosition / float(ScrHeight()) + Vec2(0, -tScale),
					(*this)[currentSelected]->color, Vec2(tScale));
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
			if (tTime2 - lastUse < bItem.useTime * shootSpeed)
				return false;
			for (Tower* tower : Defences::Towers::towers)
				if (CanMake(tower->recipe))
					return true;
			return false;
		}
		int index = currentIndex + width * currentRow;
		return (*this)[index].count != 0 && tTime2 - lastUse >= (*this)[index]->useTime * shootSpeed;
	}

	bool CanBuildOffhand(Vec3 pos, Vec3 dir)
	{
		return game->entities->RaycastEnt(pos, dir, 30, MaskF::IsAlly, player).index != -1;
	}

	bool Build(Vec3 pos, Vec3 dir)
	{
		vector<Tower*> buildableTowers;
		for (Tower* tower : Defences::Towers::towers)
			if (CanMake(tower->recipe))
				buildableTowers.push_back(tower);
		if (buildableTowers.size() == 0 || !buildableTowers[currentIndex]->TryCreate(this, pos, dir, player)) return false;
		// If we created a tower then we should make sure that currentIndex is still a valid value.
		buildableTowers.clear();
		for (Tower* tower : Defences::Towers::towers)
			if (CanMake(tower->recipe))
				buildableTowers.push_back(tower);

		if (buildableTowers.size() == 0)
			currentIndex = 0;
		else
			currentIndex = currentIndex % buildableTowers.size();

		return true;
	}

	bool BuildOffhand(Vec3 pos, Vec3 dir);

	// True means stack is empty and false means the opposite.
	bool Use(Vec3 pos, Vec3 dir)
	{
		int index = currentIndex + currentRow * width;
		ItemInstance& currentItem = (*this)[index];
		currentItem->itemU(currentItem, ProjUData(pos, dir, player));
		return RemoveIfEmpty(index);
	}

	bool CanUseOffhand(Vec3 pos, Vec3 dir, float lastUse, float shootSpeed)
	{
		int index = int(size() - 1);
		return (*this)[index].count != 0 && tTime2 - lastUse >= (*this)[index]->useTime * shootSpeed;
	}

	// True means stack is empty and false means the opposite.
	bool UseOffhand(Vec3 pos, Vec3 dir);
};

int difficultySeedSpawnQuantity[] = { 7, 5, 3 };
int difficultySeedSelectQuantity[] = { 4, 5, 6 };

#pragma region Player Types
typedef std::function<void(Player* player)> PMovement;
typedef std::function<bool(Player* player)> Primary, Offhand, Secondary, Utility;
typedef function<bool(Player* player, bool creating)> Build;

class PlayerBody : public Entity
{
public:
	PlayerBody(EntityData* data, Vec3 pos, Entity* creator) :
		Entity(data, pos, creator->radius, creator->color, creator->mass, creator->bounciness, 1, 1, creator->name + "'s body", creator->allegiance)
	{
		this->colOverlapFun = MaskF::IsCorporealNotCreator;
		this->creator = this->creator;
		Start();
	}

	HitResult ApplyHit(int damage, Entity* damageDealer) override
	{
		HitResult result = creator->ApplyHit(damage, damageDealer);
		if (result == HitResult::DIED)
			DestroySelf(damageDealer);
		return result;
	}
};

namespace Updates { void PlayerU(Entity* entity); void FlareU(Entity* entity); } namespace DUpdates { void PlayerDU(Entity* entity); }
namespace UIUpdates { void PlayerUIU(Entity* entity); void FlareUIU(Entity* entity); void EngineerUIU(Entity* entity); }
namespace OnDeaths { void PlayerOD(Entity* entity, Entity* damageDealer); }
EntityData playerData = EntityData(Updates::PlayerU, VUpdates::FrictionVU, DUpdates::PlayerDU, UIUpdates::PlayerUIU, OnDeaths::PlayerOD);
class Player : public LightBlock
{
public:
#pragma region Variables
	Base* base = nullptr;
	Entity* heldEntity = nullptr, *currentMenuedEntity = nullptr;
	PlayerInventory items;
	Items startItems;
	bool startSeeds[UnEnum(SEEDINDICES::COUNT)] = { false };
	Vec2 placingDir = north;
	float moveSpeed, maxSpeed, vacDist, vacSpeed, maxVacSpeed, shootSpeed,
		lastPrimary = 0, primaryTime, lastBuild = 0, buildTime, lastOffhand = 0, offhandTime, lastSecondary = 0, secondaryTime, lastUtility = 0, utilityTime, lastJump = 0;
	bool shouldVacuum = true, shouldPickup = true, shouldScroll = true,
		shouldRenderInventory = true;
	float timeSinceHit = 0, timeTillHeal = 0;
	EntityMaskFun vacMaskFun;
	PMovement movement;
	Primary primary;
	Build build;
	Offhand offhand;
	Secondary secondary;
	Utility utility;
	vector<TimedEvent> events{};
	float iTime = 0, sTime = 0; // Invincibility time, if <= 0 takes damage, else doesn't.
	Inputs inputs;
	float yaw = 0, pitch = 0;
	Vec3 camDir = vZero; // The direction that the camera is facing.
	Vec3 moveDir = vZero; // camDir but without the verticality.
	Vec3 rightDir = vZero; // moveDir but rotated 90 degrees to the right.
	Vec3 upDir = vZero; // Perpendicular to rightDir and moveDir.
#pragma endregion

	Player(EntityData* data, PMovement movement, Primary primary, Build build, Offhand offhand, Secondary secondary, Utility utility, EntityMaskFun vacMaskFun,
		float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8, float vacDist = 6, float vacSpeed = 16,
		float maxVacSpeed = 16, float shootSpeed = 1,
		float primaryTime = 1, float buildTime = 1, float offhandTime = 1, float secondaryTime = 1, float utilityTime = 1, RGBA color = RGBA(), RGBA color2 = RGBA(),
		JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10, float mass = 1, float bounciness = 0,
		int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}) :
		LightBlock(data, lightColor, lightOrDark, range, vZero, radius, color, color2, mass, bounciness, maxHealth, health, name, PLAYER_A),
		movement(movement), primary(primary), build(build), offhand(offhand), secondary(secondary), utility(utility), vacDist(vacDist),
		moveSpeed(moveSpeed), startItems(startItems), vacMaskFun(vacMaskFun), vacSpeed(vacSpeed), maxSpeed(maxSpeed), maxVacSpeed(maxVacSpeed),
		shootSpeed(shootSpeed), primaryTime(primaryTime), buildTime(buildTime), offhandTime(offhandTime), secondaryTime(secondaryTime), utilityTime(utilityTime)
	{
		uiActive = true;
	}

	void Start() override
	{
		LightBlock::Start(); // Set up all of the light stuff.

		iTime = 0;
		sTime = 0;

		lastPrimary = 0;
		lastBuild = 0;
		lastSecondary = 0;
		lastUtility = 0;
		shouldVacuum = true;
		items = PlayerInventory(this, 10, 4, startItems);
		if (game->settings.newBalanceChange)
		{
			this->moveSpeed *= 0.5f;
			this->maxSpeed *= 0.5f;
		}

		for (int i = 0; i < UnEnum(SEEDINDICES::COUNT); i++)
			if (startSeeds[i])
				items.push_back(Resources::Seeds::plantSeeds[i]->Clone(difficultySeedSpawnQuantity[game->difficulty]));
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

	HitResult ApplyHit(int damage, Entity* damageDealer) override
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
		return game->entities->OverlapsAny(pos + Vec3(0, 0, -radius), radius - 0.01f, MaskF::CanCollide, this);
	}

	virtual float PrimaryFill()
	{
		return items.isBuilding ? min(1.f, (tTime2 - lastBuild) / buildTime) : min(1.f, (tTime2 - lastPrimary) / (items.GetCurrentItem()->useTime * shootSpeed));
	}

	virtual float OffhandFill()
	{
		return min(1.f, (tTime2 - lastOffhand) / (items.GetCurrentOffhand()->useTime * shootSpeed));
	}

	virtual float SecondaryFill()
	{
		return min(1.f, (tTime2 - lastSecondary) / secondaryTime);
	}

	virtual float UtilityFill()
	{
		return min(1.f, (tTime2 - lastUtility) / utilityTime);
	}
};

EntityData flareData = EntityData(Updates::FlareU, VUpdates::FrictionVU, DUpdates::PlayerDU, UIUpdates::FlareUIU, OnDeaths::PlayerOD);
class Flare : public Player
{
public:
	float currentFlame, maxFlame;

	Flare(EntityData* data, float maxFlame, PMovement movement, Primary primary, Build build, Offhand offhand, Secondary secondary, Utility utility,
		EntityMaskFun vacMaskFun, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8, float vacDist = 6, float vacSpeed = 16,
		float maxVacSpeed = 16, float shootSpeed = 1, float primaryTime = 1, float buildTime = 1, float offhandTime = 1, float secondaryTime = 1, float utilityTime = 1,
		RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10, float mass = 1,
		float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}) :
		Player(data, movement, primary, build, offhand, secondary, utility, vacMaskFun, radius, moveSpeed, maxSpeed, vacDist, vacSpeed,
			maxVacSpeed, shootSpeed, primaryTime, buildTime, offhandTime, secondaryTime, utilityTime, color,
			color2, lightColor, lightOrDark, range, mass, bounciness, maxHealth, health, name, startItems),
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

enum class EngState
{
	DEFAULT, ROLLING, TURRETED
};
EntityData engineerData = EntityData(Updates::PlayerU, VUpdates::FrictionVU, DUpdates::PlayerDU, UIUpdates::EngineerUIU, OnDeaths::PlayerOD);
class Engineer : public Player
{
public:
	float rollAccelMul, rollSpeedMul, turretedShootSpeed;
	EngState engState = EngState::DEFAULT;

	Engineer(EntityData* data, PMovement movement, Primary primary, Build build, Offhand offhand, Secondary secondary, Utility utility,
		EntityMaskFun vacMaskFun, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8, float rollAccelMul = 8, float rollSpeedMul = 16,
		float vacDist = 6, float vacSpeed = 16, float maxVacSpeed = 16, float shootSpeed = 1, float turretedShootSpeed = 2, float primaryTime = 1,
		float buildTime = 1, float offhandTime = 1,
		float secondaryTime = 1, float utilityTime = 1, RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127),
		bool lightOrDark = true, float range = 10, float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1,
		string name = "NULL NAME", Items startItems = {}) :
		Player(data, movement, primary, build, offhand, secondary, utility, vacMaskFun, radius, moveSpeed, maxSpeed, vacDist, vacSpeed,
			maxVacSpeed, shootSpeed, primaryTime, buildTime, offhandTime, secondaryTime, utilityTime, color, color2,
			lightColor, lightOrDark, range, mass, bounciness, maxHealth, health, name, startItems),
		rollAccelMul(rollAccelMul), rollSpeedMul(rollSpeedMul), turretedShootSpeed(turretedShootSpeed)
	{ }

	Engineer(Engineer* baseClass, bool* startSeeds, Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) :
		Engineer(*baseClass)
	{
		this->baseClass = baseClass;
		std::copy(startSeeds, startSeeds + sizeof(bool) * UnEnum(SEEDINDICES::COUNT), this->startSeeds);
		this->pos = pos;
		this->creator = creator;
		Start();
	}

	unique_ptr<Player> PClone(bool* startSeeds, Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr)
	{
		return make_unique<Engineer>(this, startSeeds, pos, dir, creator);
	}

	float OffhandFill() override
	{
		return items.isBuilding ? Player::OffhandFill() : min(1.f, (tTime2 - lastOffhand) / (offhandTime));
	}
};

EntityData orchidData = EntityData(Updates::PlayerU, VUpdates::FrictionVU, DUpdates::PlayerDU, UIUpdates::PlayerUIU, OnDeaths::PlayerOD);
class Orchid : public Player
{
public:
	Orchid(EntityData* data, PMovement movement, Primary primary, Build build, Offhand offhand, Secondary secondary, Utility utility,
		EntityMaskFun vacMaskFun, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8, float rollAccel = 8, float rollSpeed = 16,
		float vacDist = 6, float vacSpeed = 16, float maxVacSpeed = 16, float shootSpeed = 1, float primaryTime = 1, float buildTime = 1, float offhandTime = 1,
		float secondaryTime = 1, float utilityTime = 1, RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127),
		bool lightOrDark = true, float range = 10, float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1,
		string name = "NULL NAME", Items startItems = {}) :
		Player(data, movement, primary, build, offhand, secondary, utility, vacMaskFun, radius, moveSpeed, maxSpeed, vacDist, vacSpeed,
			maxVacSpeed, shootSpeed, primaryTime, buildTime, offhandTime, secondaryTime, utilityTime, color, color2,
			lightColor, lightOrDark, range, mass, bounciness, maxHealth, health, name, startItems)
	{ }

	Orchid(Orchid* baseClass, bool* startSeeds, Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) :
		Orchid(*baseClass)
	{
		this->baseClass = baseClass;
		std::copy(startSeeds, startSeeds + sizeof(bool) * UnEnum(SEEDINDICES::COUNT), this->startSeeds);
		this->pos = pos;
		this->creator = creator;
		Start();
	}

	unique_ptr<Player> PClone(bool* startSeeds, Vec3 pos = vZero, Vec3 dir = north, Entity* creator = nullptr)
	{
		return make_unique<Orchid>(this, startSeeds, pos, dir, creator);
	}
};
#pragma endregion
#pragma region Player Creation Types
namespace Updates { void GrenadeU(Entity* entity); void FlamePuddleU(Entity* entity); void FlameGlobU(Entity* entity); void EngTurretU(Entity* entity); }
EntityData grenadeData = EntityData(Updates::GrenadeU, VUpdates::FrictionVU, DUpdates::DToColDU, UIUpdates::EntityUIU, OnDeaths::LightBlockOD);
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

EntityData flamePuddleData = EntityData(Updates::FlamePuddleU, VUpdates::EntityVU, DUpdates::FadeOutGlowDU, UIUpdates::EntityUIU, OnDeaths::FadeOutGlowOD);
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

EntityData flameGlobData = EntityData(Updates::FlameGlobU, VUpdates::FrictionVU, DUpdates::EntityDU, UIUpdates::EntityUIU, OnDeaths::LightBlockOD);
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

namespace ItemODs { void FlareFlameOD(ItemInstance& item, ProjODData data); }
class FlareFlame : public Item
{
public:
	int flameRestore;

	FlareFlame(ITEMTYPE type, int flameRestore, string name = "NULL", string typeName = "NULL TYPE", VUpdate vUpdate = VUpdates::EntityVU, int intType = 0, RGBA color = RGBA(), int damage = 1,
		float range = 15.0f, float useTime = 0.25f, float speed = 12, float radius = 0.4f, bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		Item(type, name, typeName, vUpdate, intType, color, damage, range, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health),
		flameRestore(flameRestore)
	{
		itemOD = ItemODs::FlareFlameOD;
	}
};

namespace DUpdates { void EngTurretDU(Entity* entity); }
EntityData engTurretData = EntityData(Updates::EngTurretU, VUpdates::FrictionVU, DUpdates::EngTurretDU, UIUpdates::EntityUIU, OnDeaths::LightTowerOD);
class EngTurret : public LightTower
{
public:
	ItemInstance projectiles;
	float timeTill, timePer;
	RGBA color3; // Used for the color of the barrel.

	EngTurret(EntityData* data, string* description, float timePer,
		JRGB lightColor, float range, float radius, RGBA color, RGBA color2, RGBA color3, float mass, float bounciness, int maxHealth,
		int health, string name) :
		LightTower(data, {}, description, lightColor, true, range, radius, color, color2, mass, bounciness, maxHealth, health, name),
		projectiles(dItem.Clone(0)), timePer(timePer), timeTill(timePer), color3(color3)
	{ }

	EngTurret(EngTurret* baseClass, ItemInstance projectiles, Vec3 pos, Vec3 dir, Entity* creator) :
		EngTurret(*baseClass)
	{
		this->pos = pos;
		this->dir = dir;
		this->creator = creator;
		name = creator->name + "'s " + baseClass->name;
		allegiance = creator->allegiance;
		this->projectiles = projectiles;
		Start();
	}

	unique_ptr<Entity> Clone(ItemInstance projectiles, Vec3 pos, Vec3 dir, Entity* creator)
	{
		return make_unique<EngTurret>(this, projectiles, pos, dir, creator);
	}
};
#pragma endregion
#pragma region Player Creation Instances
FlareFlame flareFlame = FlareFlame(ITEMTYPE::FLARE_FLAME, 2, "Flare Flame", "Player Creation", VUpdates::EntityVU, 0, RGBA(255, 93, 0), 10, 15, 0.0625f, 18, 1);

string engTurretStr = "Silly little goober\nIF THIS APPEARS TELL ME";
EngTurret engTurret = EngTurret(&engTurretData, &engTurretStr, 0.5f, JRGB(255, 127, 127), 5, 0.5f, RGBA(127, 63, 63), RGBA(), RGBA(127, 127, 127), 1, 0.25f, 60, 60, "Pulse Turret");
#pragma endregion
#pragma region Player Functions
namespace PMovements
{
	void Default(Player* player)
	{
		player->vel = Vec3(TryAdd2V2(player->vel, Vec2(player->inputs.MoveDir()) * game->dTime * (player->moveSpeed + game->planet->friction),
			player->maxSpeed), player->vel.z);
		if (player->inputs.keys[KeyCode::JUMP].pressed && tTime2 - player->lastJump > 0.25f &&
			player->Grounded())
		{
			player->vel.z += 7 - min(0.f, player->vel.z);
			player->lastJump = tTime2;
		}
	}

	void EngineerPM(Player* player)
	{
		Engineer* engineer = static_cast<Engineer*>(player);
		if (engineer->engState == EngState::TURRETED) return;
		if (engineer->engState == EngState::ROLLING)
		{
			float moveSpeed = engineer->moveSpeed;
			float maxSpeed = engineer->maxSpeed;
			engineer->moveSpeed *= engineer->rollAccelMul;
			engineer->maxSpeed *= engineer->rollSpeedMul;
			Default(engineer);
			engineer->moveSpeed = moveSpeed;
			engineer->maxSpeed = maxSpeed;
			return;
		}
		Default(engineer);
	}
}

namespace Primaries
{
	bool Pistol(Player* player)
	{
		if (player->items.CanUse(player->pos, player->camDir, player->lastPrimary, player->shootSpeed))
		{
			player->lastPrimary = tTime2 + player->primaryTime;
			player->items.Use(player->pos, player->camDir);
		}
		return false;
	}

	bool EngineerPr(Player* player)
	{
		Engineer* engineer = static_cast<Engineer*>(player);
		if (engineer->engState == EngState::ROLLING) return false;
		if (engineer->engState == EngState::TURRETED)
		{
			float shootSpeed = engineer->shootSpeed;
			engineer->shootSpeed = engineer->turretedShootSpeed;
			bool result = Pistol(engineer);
			engineer->shootSpeed = shootSpeed;
			return result;
		}
		return Pistol(engineer);
	}
}

namespace Builds
{
	bool Default(Player* player, bool creating)
	{
		if (!creating)
			return player->items.BuildOffhand(player->pos, player->camDir);
		return player->items.Build(player->pos, player->camDir);
	}
}

namespace Offhands
{
	bool DualWield(Player* player)
	{
		if (player->items.CanUseOffhand(player->pos, player->camDir, player->lastOffhand, player->shootSpeed))
		{
			player->lastOffhand = tTime2 + player->offhandTime;
			player->items.UseOffhand(player->pos, player->camDir);
		}
		return false;
	}

	bool MakeTurret(Player* player)
	{
		if (player->items.isBuilding) return DualWield(player);
		Engineer* engineer = static_cast<Engineer*>(player);
		ItemInstance currentItem = engineer->items[engineer->items.size() - 1];
		if (currentItem.count <= 0) return false;
		currentItem.count = min(currentItem.count, 10);
		player->items.TryTake(currentItem);
		game->entities->push_back(engTurret.Clone(currentItem, engineer->pos + engineer->camDir * (engineer->radius + engTurret.radius),
			engineer->camDir, engineer));
		return true;
	}
}

namespace Secondaries
{
	constexpr float GRENADE_THROW_SPEED = 20.f;
	bool GrenadeThrow(Player* player)
	{
		game->entities->push_back(grenade.Clone(player->pos + player->camDir * player->radius, player->camDir * GRENADE_THROW_SPEED, player));
		return true;
	}

	bool ThrowFlame(Player* player)
	{
		game->entities->push_back(flameGlob.Clone(player->pos + player->camDir * player->radius, player->camDir * GRENADE_THROW_SPEED, player));
		return true;
	}

	bool Turretify(Player* player)
	{
		Engineer* engineer = static_cast<Engineer*>(player);
		if (engineer->engState == EngState::TURRETED)
		{
			engineer->engState = EngState::DEFAULT;
			// Play sound effect or something.
		}
		else
		{
			engineer->engState = EngState::TURRETED;
			// Play different sound effect or something.
		}
		engineer->sTime += engineer->secondaryTime;
		return true;
	}
}

namespace Utilities
{
	bool TacticoolRoll(Player* player)
	{
		constexpr float TACTICOOL_ROLL_SPEED = 15.f;
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
		temp->itemU(temp, ProjUData(flare->pos, flare->dir, flare));
		return true;
	}

	bool Rollify(Player* player)
	{
		Engineer* engineer = static_cast<Engineer*>(player);
		if (engineer->engState == EngState::ROLLING)
		{
			engineer->engState = EngState::DEFAULT;
			// Play sound effect or something.
		}
		else
		{
			engineer->engState = EngState::ROLLING;
			// Play different sound effect or something.
		}
		engineer->sTime += engineer->utilityTime;
		return true;
	}
}
#pragma endregion
#pragma region Player Instances
Player soldier = Player(&playerData, PMovements::Default, Primaries::Pistol, Builds::Default, Offhands::DualWield, Secondaries::GrenadeThrow,
	Utilities::TacticoolRoll, MaskF::IsCollectible, 0.4f, 32, 8, 6, 256, 32, 1, 0, 0.25f, 0, 2, 4, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), true, 20, 5, 0.25f, 100, 50,
	"Soldier", Items({ Resources::iron.Clone(10) }));

Flare flare = Flare(&flareData, 100, PMovements::Default, Primaries::Pistol, Builds::Default, Offhands::DualWield, Secondaries::ThrowFlame,
	Utilities::FlameThrower, MaskF::IsCollectible, 0.4f, 32, 8, 6, 256, 32, 1, 0, 0.25f, 0, 2, 0.0625f, RGBA(255, 255), RGBA(0, 0, 255),
	JRGB(127, 127, 127), true, 5.f, 1.5f, 0.25f, 100, 50, "Flare", Items({ Resources::rock.Clone(10) }));

Engineer engineer = Engineer(&engineerData, PMovements::EngineerPM, Primaries::EngineerPr, Builds::Default, Offhands::MakeTurret, Secondaries::Turretify,
	Utilities::Rollify, MaskF::IsCollectible, 0.4f, 32, 8, 1, 2, 6, 256, 32, 1, 0.25f, 0, 0.25f, 1, 0.5f, 0.25f, RGBA(127, 63, 63), RGBA(), JRGB(127, 127, 127),
	true, 20, 5, 0.25f, 100, 50, "Engineer", Items({ Resources::copper.Clone(30) }));

vector<Player*> characters = { &soldier, &flare, &engineer };
#pragma endregion
#pragma region Base Types
namespace Updates { void BaseU(Entity* entity); } namespace OnDeaths { void BaseOD(Entity* entity, Entity* damageDealer); }
EntityData baseData = EntityData(Updates::BaseU, VUpdates::FrictionVU, DUpdates::DToColDU, UIUpdates::EntityUIU, OnDeaths::BaseOD);
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

	HitResult ApplyHit(int damage, Entity* damageDealer) override
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

		if (!game->settings.newBalanceChange)
			LightBlockOD(player->base, damageDealer);
		LightBlockOD(player, damageDealer);
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

		LightBlockOD(base->player, damageDealer);
		LightBlockOD(base, damageDealer);
		playerAlive = false;
		deathName = base->player->name + "'s base";
		if (damageDealer != nullptr)
			deathCauseName = damageDealer->name;
		else
			deathCauseName = "The planet";
	}
}

namespace ItemODs
{
	void FlareFlameOD(ItemInstance& item, ProjODData data)
	{
		if (data.hitType != HitResult::MISSED && game->player && game->player->name == "Flare")
			static_cast<Flare*>(game->player)->currentFlame += static_cast<FlareFlame*>(item.Type())->flameRestore;
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

			int tempCurrentIndex = currentIndex2;
			currentIndex2 = currentIndex;
			currentIndex = tempCurrentIndex;

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
		int tempCurrentIndex = currentIndex2;
		currentIndex2 = currentIndex;
		currentIndex = tempCurrentIndex;
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
			int newSelected = int(size() - 1);
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

bool PlayerInventory::BuildOffhand(Vec3 pos, Vec3 dir)
{
	RaycastHit hit = game->entities->RaycastEnt(pos, player->dir, 30, MaskF::IsAlly, player);
	if (hit.index == -1) return false;
	Entity* hitEntity = (*game->entities)[hit.index].get();
	if (hitEntity != static_cast<Entity*>(game->base))
		return hitEntity->ApplyHit(100000, player) == HitResult::DIED;
}

bool PlayerInventory::UseOffhand(Vec3 pos, Vec3 dir)
{
	int index = int(size() - 1);
	ItemInstance& currentItem = (*this)[index];
	currentItem->itemU(currentItem, ProjUData(pos, dir, player));
	return RemoveIfEmpty(index);
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

		// Mouse movement stuff:
		Vec2 mouseOffset = player->items.isOpen ? vZero : game->inputs.mouseOffset;
		player->yaw -= mouseOffset.x;
		player->pitch = ClampF(player->pitch + mouseOffset.y, -89, 89);
		// Refind the directions for the player:
		player->camDir = Vec3(
			cos(glm::radians(player->yaw)) * cos(glm::radians(player->pitch)),
			sin(glm::radians(player->yaw)) * cos(glm::radians(player->pitch)),
			sin(glm::radians(player->pitch)));
		player->camDir = glm::normalize(player->camDir);
		player->moveDir = glm::normalize(Vec3(Vec2(player->camDir), 0));
		player->rightDir = glm::normalize(glm::cross(player->camDir, up));
		player->upDir = glm::normalize(glm::cross(player->rightDir, player->camDir));
		player->dir = player->camDir;

		if (player->sTime <= 0 && game->updateMode == UPDATEMODE::IN_GAME)
		{
			player->movement(player); // This handles all of the locomotion of the player.

			ItemInstance currentShootingItem = player->items.GetCurrentItem();

			if (player->items.isBuilding)
			{
				if ((player->inputs.keys[KeyCode::PRIMARY].held || player->inputs.keys[KeyCode::OFFHAND].held)
					&& tTime2 - player->lastBuild >= player->buildTime && player->build(player, player->inputs.keys[KeyCode::PRIMARY].held))
					player->lastBuild = tTime2;
			}
			else if (!player->items.isOpen)
			{
				if (player->inputs.keys[KeyCode::PRIMARY].held && tTime2 - player->lastPrimary >= player->primaryTime && player->primary(player))
					player->lastPrimary = tTime2;
				if (player->inputs.keys[KeyCode::OFFHAND].held && tTime2 - player->lastOffhand >= player->offhandTime && player->offhand(player))
					player->lastOffhand = tTime2;
			}
			if (player->inputs.keys[KeyCode::SECONDARY].held && tTime2 - player->lastSecondary >= player->secondaryTime && player->secondary(player))
				player->lastSecondary = tTime2;
			if (player->inputs.keys[KeyCode::UTILITY].held && tTime2 - player->lastUtility >= player->utilityTime && player->utility(player))
				player->lastUtility = tTime2;
			
			 if (player->shouldVacuum)
				 game->entities->Vacuum(player->pos, player->vacDist, player->vacSpeed, player->maxVacSpeed, player->vacMaskFun, player);
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
		flare->currentFlame = ClampF(flare->currentFlame - game->dTime * 0.5f, 0, flare->maxFlame);
		float tempSpeed = flare->maxSpeed;
		flare->maxSpeed *= flare->currentFlame / flare->maxFlame;
		flare->maxSpeed = Lerp(flare->maxSpeed, tempSpeed, 0.5f);
		PlayerU(entity);
		flare->maxSpeed = tempSpeed;
	}

	void GrenadeU(Entity* entity)
	{
		Grenade* grenade = static_cast<Grenade*>(entity);
		if (tTime - grenade->startTime > grenade->timeTill)
		{
			CreateUpExplosion(grenade->pos, grenade->explosionRadius, grenade->color, grenade->name, 0, grenade->damage, grenade->creator);
			game->entities->VacuumBurst(grenade->pos, grenade->explosionRadius, -grenade->pushForce, 50.f, MaskF::IsDifferent, grenade);
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
					e->statuses.push_back(StatusEffect(e, STATUS::FLARE_FIRE, e == creator ? 3.f : 1.f));

				creator->currentFlame++;
			}
		}
		FadeOutU(entity);
	}

	void EngTurretU(Entity* entity)
	{
		EngTurret* turret = static_cast<EngTurret*>(entity);
		turret->timeTill -= game->dTime;
		Entity* hitEntity = nullptr;
		if (turret->timeTill <= 0 && (hitEntity =
			game->entities->ExtremestOverlap(turret->pos, turret->projectiles->range, MaskF::IsNonAlly, ExtrF::SqrDist, turret).first) != nullptr)
		{
			turret->dir = Normalized(hitEntity->pos - turret->pos);
			turret->projectiles->itemU(turret->projectiles, ProjUData(turret->pos, turret->dir, turret->creator));
			if (turret->projectiles.count <= 0)
				return turret->DestroySelf(nullptr);
			turret->timeTill = turret->timePer * turret->projectiles->useTime;
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

	void EngTurretDU(Entity* entity)
	{
		EngTurret* turret = static_cast<EngTurret*>(entity);

		game->DrawCylinder(turret->pos, turret->pos + turret->radius * 2 * turret->dir, turret->color3, turret->radius * 0.25f);
		game->DrawCylinder(turret->pos, turret->pos + turret->radius * (1 + 0.1f * turret->projectiles.count) * up, turret->projectiles->color,
			turret->radius * 0.25f);

		DToColDU(entity);
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
		Vec2 offset = iVec2(ScrWidth(), -scrHeight);
		offset.x -= scale * 2;
		game->DrawFBL(offset, RGBA(0, 0, 0), Vec2(scale));
		game->DrawFBL(offset, RGBA(255, 0, 0), Vec2(scale, scale * player->UtilityFill()));
		offset.x -= scale * 2;
		game->DrawFBL(offset, RGBA(0, 0, 0), Vec2(scale));
		game->DrawFBL(offset, RGBA(255, 255, 0), Vec2(scale, scale * player->SecondaryFill()));
		offset.x -= scale * 2;
		game->DrawFBL(offset, RGBA(0, 0, 0), Vec2(scale));
		game->DrawFBL(offset, RGBA(0, 255, 0), Vec2(scale, scale * player->PrimaryFill()));
		offset.x -= scale * 2;
		game->DrawFBL(offset, RGBA(0, 0, 0), Vec2(scale));
		game->DrawFBL(offset, RGBA(0, 0, 255), Vec2(scale, scale * player->OffhandFill()));

		// Reticle:
		game->DrawTextured(reticleSprite, 0, vZero, -Vec2(1.f / 24),
			RGBA(255, 255, 255, 127), Vec2(1.f / 12));
	}

	void FlareUIU(Entity* entity)
	{
		Flare* flare = static_cast<Flare*>(entity);
		if (!game->showUI) return;
		PlayerUIU(entity);
		float scale = ScrHeight() / 30.f;
		Vec2 offset = Vec2(ScrWidth() - scale * 8, scale * 2 - ScrHeight());
		game->DrawFBL(offset, RGBA(18, 156, 163), Vec2(scale * 4, scale));
		game->DrawFBL(offset, RGBA(163, 71, 18), Vec2(4 * scale * flare->currentFlame / flare->maxFlame, scale));
		vector<StatusEffect>::iterator itr;
		if ((itr = std::find_if(flare->statuses.begin(), flare->statuses.end(), [](StatusEffect const& s) { return s.status == STATUS::FLARE_FIRE; })) != flare->statuses.end())
		{
			offset.y += scale * 2;
			float ratio = 0.33333f * itr->timeTillRemove;
			game->DrawFBL(offset, RGBA(41, 45, 74, byte(ratio * 255)), Vec2(scale * 4, scale));
			game->DrawFBL(offset, RGBA(15, 35, 166).CLerp(RGBA(104, 201, 212), 0.5f + 0.5f * sinf(tTime * PI_F * 3)).CLerp(
				RGBA(15, 35, 166).CLerp(RGBA(104, 201, 212), 0.5f), 1 - ratio), Vec2(4 * scale * ratio, scale));
		}
	}

	void EngineerUIU(Entity* entity)
	{
		Engineer* engineer = static_cast<Engineer*>(entity);
		if (!game->showUI) return;
		PlayerUIU(entity);
		float scale = ScrHeight() / 30.f;
		iVec2 offset = iVec2(ScrWidth() - scale * 8, scale * 2 - ScrHeight());
		switch (engineer->engState)
		{
		case EngState::DEFAULT: break;
		case EngState::ROLLING:
			game->DrawFBL(offset, RGBA(146, 171, 126), Vec2(scale * 4, scale));
			break;
		case EngState::TURRETED:
			game->DrawFBL(offset, RGBA(59, 54, 33), Vec2(scale * 4, scale));
			break;
		}
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

	constexpr int FLARE_FIRE_SPD = 2;
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