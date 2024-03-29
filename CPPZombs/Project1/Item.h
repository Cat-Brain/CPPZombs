#include "StatusEffect.h"

enum class ITEMTYPE : byte
{
	// Internal
	DITEM, BITEM,
	// Ammo
	COPPER, IRON, ROCK, BOWLER, SILVER, RUBY, EMERALD, TOPAZ, SAPPHIRE, LEAD, VACUUMIUM, QUARTZ, BRICK, CHEESE, SHADE, GRUBIUM,
	// Tree seeds:
	COPPER_SHRUB_SEED, IRON_SHRUB_SEED, ROCK_SHRUB_SEED, RUBY_SHRUB_SEED, EMERALD_SHRUB_SEED,
	SHADE_SHRUB_SEED, BOWLER_SHRUB_SEED, VACUUMIUM_SHRUB_SEED, SILVER_SHRUB_SEED, QUARTZ_SHRUB_SEED,
	COAL, BRICK_SHRUB_SEED, GRUBIUM_SHRUB_SEED,
	// Vine seeds:
	CHEESE_VINE_SEED, TOPAZ_VINE_SEED, SAPPHIRE_VINE_SEED, LEAD_VINE_SEED,
	// Enemy items:
	GIGA_TANK_ITEM,
	// Other:
	KIWI_EGG,
	// Player items:
	FLARE_FLAME,
	// Count:
	COUNT
};
class Item;
vector<Item*> items = vector<Item*>(UnEnum(ITEMTYPE::COUNT), nullptr);

class ItemInstance
{
public:
	ITEMTYPE type;
	int count;

	ItemInstance(ITEMTYPE type = ITEMTYPE::DITEM, int count = 1) : type(type), count(count) { }


	inline Item* Type()
	{
		return items[UnEnum(type)];
	}

	inline Item* operator->()
	{
		return Type();
	}

	inline ItemInstance Clone(uint count)
	{
		return ItemInstance(type, count);
	}

	inline ItemInstance Clone()
	{
		return Clone(count);
	}

	inline ItemInstance operator * (int multiplier)
	{
		return Clone(count * multiplier);
	}

	inline bool operator == (ItemInstance b)
	{
		return Type() == b.Type();
	}

	inline bool operator != (ItemInstance b)
	{
		return Type() != b.Type();
	}

	inline bool operator == (ITEMTYPE type)
	{
		return type == type;
	}

	inline bool operator != (ITEMTYPE type)
	{
		return type != type;
	}

	inline bool operator < (ItemInstance b)
	{
		return Type() < b.Type();
	}
};

struct ProjUData
{
	Vec3 pos, dir;
	Entity* creator;

	ProjUData(Vec3 pos, Vec3 dir, Entity* creator) :
		pos(pos), dir(dir), creator(creator) { }
};

struct ProjODFlags
{
	bool usedAmmo : 1; // 1
	bool b : 1; //        2
	bool c : 1; //        4
	bool d : 1; //        8
	bool e : 1; //       16
	bool f : 1; //       32
	bool g : 1; //       64
	bool h : 1; //      128

	ProjODFlags(byte value = 0)
	{
		*this = value; // Uses custom = operator;
	}

	inline void operator = (byte value)
	{
		*(byte*)this = value;
	}

	inline ProjODFlags operator | (ProjODFlags other)
	{
		return *((byte*)this) | *((byte*)&other);
	}

	inline ProjODFlags operator + (ProjODFlags other)
	{
		return *this | other;
	}

	inline byte OverlapByte(ProjODFlags other)
	{
		return *((byte*)this) & *((byte*)&other);
	}

	inline ProjODFlags Overlap(ProjODFlags other)
	{
		return ProjODFlags(OverlapByte(other));
	}

	inline bool HasOverlap(ProjODFlags other)
	{
		return OverlapByte(other) != 0;
	}
};

struct ProjODData
{
	Vec3 pos, dir, vel;
	Entity* creator;
	string creatorName;
	Entity* callReason;
	HitResult hitType;
	ProjODFlags flags;

	ProjODData(Vec3 pos, Vec3 dir, Vec3 vel, Entity* creator, string creatorName, Entity* callReason, HitResult hitType, ProjODFlags flags) :
		pos(pos), dir(dir), vel(vel), creator(creator), creatorName(creatorName), callReason(callReason), hitType(hitType), flags(flags) { }
};

typedef function<void(ItemInstance& item, ProjUData data)> ItemU;

typedef function<void(ItemInstance& item, ProjODData data)> ItemOD; // All item on-death effects.

namespace ItemUs { void DefaultU(ItemInstance& item, ProjUData data); }
namespace ItemODs { void DefaultOD(ItemInstance& item, ProjODData data);
	void GoneOnLandItemOD(ItemInstance& item, ProjODData data); }
class Item
{
public:
	ITEMTYPE type;
	ItemU itemU;
	ItemOD itemOD;

	int maxStack;
	string name;
	string typeName;
	int intType; // Looked up for sprite.
	RGBA color;
	int damage;
	float range, useTime, speed;
	float radius;
	bool corporeal; // Is this corporeal when shot? Ignored by collectible which is never corporeal.
	bool shouldCollide; // Should this as a projectile be destroyed upon contact?
	bool collideTerrain;
	float mass;
	int health;
	VUpdate vUpdate;

	Item(ITEMTYPE type, string name, string typeName, VUpdate vUpdate, int intType = 0, RGBA color = RGBA(), int damage = 1,
		float range = 15.0f, float useTime = 0.25f, float speed = 12, float radius = 0.4f, bool corporeal = false, bool shouldCollide = true,
		bool collideTerrain = false, float mass = 1, int health = 1) :
		type(type), itemU(ItemUs::DefaultU), itemOD(ItemODs::DefaultOD), maxStack(99999), name(name), typeName(typeName), vUpdate(vUpdate), intType(intType),
		color(color), damage(damage), range(range), useTime(useTime), speed(speed), radius(radius), corporeal(corporeal), shouldCollide(shouldCollide),
		collideTerrain(collideTerrain), mass(mass), health(health)
	{
		items[UnEnum(type)] = this;
	}

	ItemInstance Clone(int count = 1)
	{
		return ItemInstance(type, count);
	}

	bool operator == (Item b)
	{
		return name == b.name;
	}

	bool operator != (Item b)
	{
		return name != b.name;
	}

	bool operator <(Item b)
	{
		return typeName[0] < b.typeName[0];
	}
};

namespace VUpdates { void EntityVU(Entity* entity); void GravityVU(Entity* entity); }

Item dItem = Item(ITEMTYPE::DITEM, "NULL", "NULL TYPE", VUpdates::EntityVU, 0, RGBA(), 0);
Item bItem = Item(ITEMTYPE::BITEM, "BUILD", "NULL TYPE", VUpdates::EntityVU, 0, RGBA(), 0);


class GoneOnLandItem : public Item
{
public:
	GoneOnLandItem(ITEMTYPE type, string name = "NULL", string typeName = "NULL TYPE", VUpdate vUpdate = VUpdates::EntityVU, int intType = 0, RGBA color = RGBA(), int damage = 1,
		float range = 15.0f, float useTime = 0.25f, float speed = 12, float radius = 0.4f, bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, float mass = 1, int health = 1) :
		Item(type, name, typeName, vUpdate, intType, color, damage, range, useTime, speed, radius, corporeal, shouldCollide, collideTerrain, mass, health)
	{
		itemOD = ItemODs::GoneOnLandItemOD;
	}
};

void ItemODs::GoneOnLandItemOD(ItemInstance& item, ProjODData data) { }

typedef vector<ItemInstance> Recipe;

namespace TRYTAKE
{
	byte DECREMENTED = 0, DELETED = 1, TO_FEW = 2, UNFOUND = 3,
		SUCCESS = DELETED, FAILURE = TO_FEW; // TryMake <= SUCCESS  TryMake >= FAILURE

	inline bool Success(byte value) { return value <= SUCCESS; }
	inline bool Failure(byte value) { return value >= FAILURE; }
}

class Items : public vector<ItemInstance> // Vector of ItemInstance with helper functions
{
public:
	using vector<ItemInstance>::vector;

	int currentIndex = 0;

	bool RemoveIfEmpty(int index)
	{
		if ((*this)[index].count > 0)
			return false;

		erase(begin() + index);
		if (index >= currentIndex)
			currentIndex = max(0, currentIndex - 1);
		return true;
	}

	void push_back(ItemInstance item)
	{
		for (int i = 0; i < size(); i++)
			if ((*this)[i] == item)
			{
				(*this)[i].count += item.count;
				(*this)[i].count = min((*this)[i].count, (*this)[i]->maxStack);
				return;
			}
		vector<ItemInstance>::push_back(item.Clone());
		std::sort(begin(), end());
	}

	virtual byte TryTake(ItemInstance item)
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
				erase(begin() + i);
				if (i >= currentIndex)
					currentIndex = max(0, currentIndex - 1);
				return TRYTAKE::DELETED;
			}
		}
		return TRYTAKE::UNFOUND;
	}

	bool TryTakeIndex(int index)
	{
		if (index >= size() || index < 0)
			return false;
		if ((*this)[index].count == 1)
		{
			erase(begin() + index);
			if (index >= currentIndex)
				currentIndex = max(0, currentIndex - 1);
		}
		else
			(*this)[index].count -= 1;
		return true;
	}

	bool TryTakeIndex(int index, ItemInstance& result)
	{
		if (index >= size() || index < 0)
			return false;

		result = (*this)[index];
		if ((*this)[index].count == 1)
		{
			erase(begin() + index);
			if (index >= currentIndex)
				currentIndex--;
		}
		else
			(*this)[index].count -= 1;
		return true;
	}

	virtual bool CanMake(Recipe cost)
	{
		Items clone = *this;
		for (ItemInstance item : cost)
			if (TRYTAKE::Failure(clone.TryTake(item)))
				return false;
		return true;
	}

	virtual bool TryMake(Recipe cost)
	{
		Items clone = *this;
		for (ItemInstance item : cost)
			if (TRYTAKE::Failure(clone.TryTake(item)))
				return false;
		*this = clone;
		return true;
	}

	virtual ItemInstance GetCurrentItem()
	{
		if (size() > 0)
			return (*this)[currentIndex].Clone(1);
		return ItemInstance(ITEMTYPE::DITEM, 0);
	}
};

namespace Resources
{
	Item copper = Item(ITEMTYPE::COPPER, "Copper", "Ammo", VUpdates::EntityVU, 1, RGBA(232, 107, 5), 10);
	Item iron = Item(ITEMTYPE::IRON, "Iron", "Ammo", VUpdates::EntityVU, 1, RGBA(111, 123, 128), 120, 15, 1);
	Item rock = Item(ITEMTYPE::ROCK, "Rock", "Ammo", VUpdates::EntityVU, 1, RGBA(145, 141, 118), 60, 5);
	Item bowler = Item(ITEMTYPE::BOWLER, "Bowler", "Push Ammo", VUpdates::GravityVU, 1, RGBA(36, 15, 110), 0, 15.f, 0.5f, 6.f, 2.5f, true, false, false, 25, 600);
	Item silver = Item(ITEMTYPE::SILVER, "Silver", "Ammo", VUpdates::EntityVU, 1, RGBA(197, 191, 214), 30, 15.f, 0.25f, 24.f, 0.2f);
}