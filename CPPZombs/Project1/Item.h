#include "Planet.h"

class Item;

enum ITEMU // Item use functions
{
	DITEMU, WAVEMODIFIERU
};

vector<function<void(Item* stack, Vec2 pos, Vec2 dir, Entity* creator, string creatorName, Entity* callReason, int callType)>> itemUs; // Returns if should be consumed

enum ITEMOD // Item on-deaths
{
	DITEMOD, GONEONLANDITEMOD, PLACEDONLANDINGOD, CORRUPTONKILLOD, EXPLODEONLANDINGOD
};

vector<function<void(Item* item, Vec2 pos, Vec2 dir, Entity* creator, string creatorName, Entity* callReason, int callType)>> itemODs; // All item on-death effects.

enum PROJMOVE // The movements of projectile
{
	DEFAULT, HOMING
};
class Item
{
public:
	ITEMU itemU;
	ITEMOD itemOD;
	int maxStack;
	Item* baseClass;
	string name;
	string typeName;
	int intType;
	RGBA color;
	int damage;
	int count;
	float range, useTime;
	float radius;
	bool corporeal; // Is this corporeal when shot? Ignored by collectible which is never corporeal.
	bool shouldCollide; // Should this as a projectile be destroyed upon contact?
	float mass;
	int health;

	Item(string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), int damage = 1,
		int count = 1, float range = 15.0f, float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		itemU(ITEMU::DITEMU), itemOD(ITEMOD::DITEMOD), maxStack(99999), baseClass(this), name(name), typeName(typeName), intType(intType), color(color), damage(damage), count(count),
		range(range), useTime(useTime), radius(radius), corporeal(corporeal), shouldCollide(shouldCollide), mass(mass), health(health) { }

	Item(Item* baseClass, string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(),
		int damage = 1, int count = 1, float range = 15.0f, float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		itemU(ITEMU::DITEMU), itemOD(ITEMOD::DITEMOD), maxStack(99999), baseClass(baseClass), name(name), typeName(typeName), intType(intType), color(color), damage(damage), count(count),
		range(range), useTime(useTime), radius(radius), corporeal(corporeal), shouldCollide(shouldCollide), mass(mass), health(health) { }

	virtual Item Clone(int count)
	{
		return Item(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health);
	}

	Item Clone()
	{
		return Clone(count);
	}

	virtual Item* Clone2(int count)
	{
		return new Item(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2()
	{
		return Clone2(count);
	}

	Item operator * (int multiplier)
	{
		return Clone(count * multiplier);
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

	void OnDeath(ITEMOD itemOD, Vec2 pos, Vec2 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		itemODs[itemOD](this, pos, dir, creator, creatorName, callReason, callType);
	}

	void OnDeath(Vec2 pos, Vec2 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		OnDeath(itemOD, pos, dir, creator, creatorName, callReason, callType);
	}

	void Use(ITEMU itemU, Vec2 pos, Vec2 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		itemUs[itemU](this, pos, dir, creator, creatorName, callReason, callType);
	}

	void Use(Vec2 pos, Vec2 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		Use(baseClass->itemU, pos, dir, creator, creatorName, callReason, callType);
	}
};
Item* dItem = new Item("NULL", "NULL TYPE", 0, RGBA(), 0, 0);

class GoneOnLandItem : public Item
{
public:
	GoneOnLandItem(string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), int damage = 1,
		int count = 1, float range = 15.0f, float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::GONEONLANDITEMOD;
	}

	GoneOnLandItem(Item* baseClass, string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(),
		int damage = 1, int count = 1, float range = 15.0f, float useTime = 0.25f, float radius = 0.5f, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::GONEONLANDITEMOD;
	}

	Item Clone(int count) override
	{
		return GoneOnLandItem(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2(int count) override
	{
		return new GoneOnLandItem(baseClass, name, typeName, intType, color, damage, count, range, useTime, radius, corporeal, shouldCollide, mass, health);
	}
};

namespace ItemODs
{
	void GoneOnLandItemOD(Item* item, Vec2 pos, Vec2 dir, Entity* creator, string creatorName, Entity* callReason, int callType) { }
}

class Items : public vector<Item>
{
public:
	using vector<Item>::vector;

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

	void push_back(Item item)
	{
		for (int i = 0; i < size(); i++)
			if ((*this)[i].name == item.name)
			{
				(*this)[i].count += item.count;
				(*this)[i].count = min((*this)[i].count, (*this)[i].maxStack);
				return;
			}
		vector<Item>::push_back(item.Clone());
		std::sort(begin(), end());
	}

	bool TryTake(Item item)// Count should be positive.
	{
		for (int i = 0; i < size(); i++)
		{
			if ((*this)[i].name == item.name)
			{
				if ((*this)[i].count - item.count < 0)
					return false;
				if ((*this)[i].count == item.count)
				{
					erase(begin() + i);
					if (i >= currentIndex)
						currentIndex = max(0, currentIndex - 1);
				}
				else
					(*this)[i].count -= item.count;
				return true;
			}
		}
		return false;
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

	bool TryTakeIndex(int index, Item& result)
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

	bool TryMake(vector<Item> cost)
	{
		Items clone = *this;
		for (Item item : cost)
		{
			bool successful = clone.TryTake(item);
			if (!successful)
				return false;
		}
		*this = clone;
		return true;
	}

	void DUpdate()
	{
		if (size() == 0)
			return;
		int height = ScrHeight();
		float scale = height / (3.0f * max(8, int(size()))), scale2 = scale / 5.0f;
		iVec2 offset = vZeroI - ScrDim();
		game->DrawFBL(offset + iVec2(0, static_cast<int>(scale * currentIndex * 2)), (*this)[currentIndex].color, Vec2(scale, scale));
		for (int i = 0; i < size(); i++)
		{
			game->DrawTextured(spriteSheet, (*this)[i].intType, offset + iVec2(0, static_cast<int>(scale * i * 2)),
				i == currentIndex ? RGBA() : (*this)[i].color, Vec2(scale, scale));
			font.Render(" " + (*this)[i].name + "  " + to_string((*this)[i].count) + "  " + (*this)[i].typeName,
				iVec2(static_cast<int>(-ScrWidth() + scale * 2), static_cast<int>(-ScrHeight() + scale * 2 * i)), scale * 2, (*this)[i].color);
		}
	}

	Item GetCurrentItem()
	{
		if (size() > 0)
			return (*this)[currentIndex].Clone(1);
		return dItem;
	}
};

namespace Resources
{
	Item* copper = new Item("Copper", "Ammo", 1, RGBA(232, 107, 5), 1);
	GoneOnLandItem* iron = new GoneOnLandItem("Iron", "Ammo", 1, RGBA(111, 123, 128), 12, 1, 15, 0.125f);
	Item* rock = new Item("Rock", "Ammo", 1, RGBA(145, 141, 118), 6, 1, 5);
	Item* bowler = new Item("Bowler", "Push Ammo", 1, RGBA(36, 15, 110), 0, 1, 15, 0.5f, 2.5f, true, false, 25, 5);
}