#include "Game1.h"

enum ITEMOD // Item on-deaths
{
	ITEM, GONEONLANDITEM, PLACEDONLANDING, CORRUPTONKILL, EXPLODEONLANDING
};

class Item;
vector<function<void(Item* item, Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType)>> itemODs; // All item on-death effects.

class Item
{
public:
	ITEMOD itemOD;
	Item* baseClass;
	string name;
	string typeName;
	int intType;
	RGBA color, subScat;
	int damage;
	int count;
	float range, shootSpeed;
	Vec2f dimensions;
	bool corporeal; // Is this corporeal when shot? Ignored by collectible which is never corporeal.
	bool shouldCollide; // Should this as a projectile be destroyed upon contact?
	float mass;
	int health;

	Item(string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), RGBA subScat = RGBA(), int damage = 1,
		int count = 1, float range = 15.0f, float shootSpeed = 0.25f, Vec2f dimensions = vOne, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		itemOD(ITEMOD::ITEM), baseClass(this), name(name), typeName(typeName), intType(intType), color(color), subScat(subScat), damage(damage), count(count),
		range(range), shootSpeed(shootSpeed), dimensions(dimensions), corporeal(corporeal), shouldCollide(shouldCollide), mass(mass), health(health) { }

	Item(Item* baseClass, string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), RGBA subScat = RGBA(),
		int damage = 1, int count = 1, float range = 15.0f, float shootSpeed = 0.25f, Vec2f dimensions = vOne, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		itemOD(ITEMOD::ITEM), baseClass(baseClass), name(name), typeName(typeName), intType(intType), color(color), subScat(subScat), damage(damage), count(count),
		range(range), shootSpeed(shootSpeed), dimensions(dimensions), corporeal(corporeal), shouldCollide(shouldCollide), mass(mass), health(health) { }

	virtual Item Clone(int count)
	{
		return Item(baseClass, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, dimensions, corporeal, shouldCollide, mass, health);
	}

	Item Clone()
	{
		return Clone(count);
	}

	virtual Item* Clone2(int count)
	{
		return new Item(baseClass, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, dimensions, corporeal, shouldCollide, mass, health);
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

	void OnDeath(ITEMOD itemOD, Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		itemODs[itemOD](this, pos, creator, creatorName, callReason, callType);
	}

	void OnDeath(Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		OnDeath(itemOD, pos, creator, creatorName, callReason, callType);
	}
};
Item* dItem = new Item("NULL", "NULL TYPE", 0, RGBA(), 0, 0);

class GoneOnLandItem : public Item
{
public:
	GoneOnLandItem(string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), RGBA subScat = RGBA(), int damage = 1,
		int count = 1, float range = 15.0f, float shootSpeed = 0.25f, Vec2f dimensions = vOne, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(name, typeName, intType, color, subScat, damage, count, range, shootSpeed, dimensions, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::GONEONLANDITEM;
	}

	GoneOnLandItem(Item* baseClass, string name = "NULL", string typeName = "NULL TYPE", int intType = 0, RGBA color = RGBA(), RGBA subScat = RGBA(),
		int damage = 1, int count = 1, float range = 15.0f, float shootSpeed = 0.25f, Vec2f dimensions = vOne, bool corporeal = false, bool shouldCollide = true, float mass = 1, int health = 1) :
		Item(baseClass, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, dimensions, corporeal, shouldCollide, mass, health)
	{
		itemOD = ITEMOD::GONEONLANDITEM;
	}

	Item Clone(int count) override
	{
		return GoneOnLandItem(baseClass, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, dimensions, corporeal, shouldCollide, mass, health);
	}

	Item* Clone2(int count) override
	{
		return new GoneOnLandItem(baseClass, name, typeName, intType, color, subScat, damage, count, range, shootSpeed, dimensions, corporeal, shouldCollide, mass, health);
	}
};

namespace ItemODs
{
	void GoneOnLandItemOD(Item* item, Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType) { }
}

class Items : public vector<Item>
{
public:
	using vector<Item>::vector;

	int currentIndex = 0;

	void push_back(Item item)
	{
		for (int i = 0; i < size(); i++)
			if ((*this)[i].name == item.name)
			{
				(*this)[i].count += item.count;
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
		float scale = height / (2.0f * max(8, int(size()))), scale2 = scale / 5.0f;
		Vec2
			offset = vZero - ScrDim();
		game->DrawFBL(offset + Vec2(0, static_cast<int>(scale * currentIndex * 2)), (*this)[currentIndex].color, Vec2f(scale, scale));
		for (int i = 0; i < size(); i++)
		{
			game->DrawTextured(spriteSheet, (*this)[i].intType, offset + Vec2(0, static_cast<int>(scale * i * 2)),
				i == currentIndex ? RGBA() : (*this)[i].color, Vec2f(scale, scale));
			font.Render(" " + (*this)[i].name + "  " + to_string((*this)[i].count) + "  " + (*this)[i].typeName,
				Vec2(static_cast<int>(-ScrWidth() + scale * 2), static_cast<int>(-ScrHeight() + scale * 2 * i)), scale * 2, (*this)[i].color);
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
	Item* copper = new Item("Copper", "Ammo", 1, RGBA(232, 107, 5), RGBA(0, 5, 10), 1);
	GoneOnLandItem* iron = new GoneOnLandItem("Iron", "Ammo", 1, RGBA(111, 123, 128), RGBA(10, 10, 5), 12, 1, 15, 0.125f);
	Item* rock = new Item("Rock", "Ammo", 1, RGBA(145, 141, 118), RGBA(5, 5, 10), 6, 1, 5);
	Item* bowler = new Item("Bowler", "Push Ammo", 1, RGBA(36, 15, 110), RGBA(5, 0, 10), 0, 1, 15, 0.5f, vOne * 5, true, false, 25, 5);
}