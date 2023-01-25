#include "Game1.h"

class Item
{
public:
	Item* baseClass;
	string name;
	string typeName;
	Color color;
	int damage;
	int count;
	float range;
	Vec2 dimensions;

	Item(string name = "NULL", string typeName = "NULL TYPE", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f, Vec2 dimensions = vOne) :
		baseClass(this), name(name), typeName(typeName), color(color), damage(damage), count(count), range(range), dimensions(dimensions) { }

	Item(Item* baseClass, string name = "NULL", string typeName = "NULL TYPE", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f, Vec2 dimensions = vOne) :
		baseClass(baseClass), name(name), typeName(typeName), color(color), damage(damage), count(count), range(range), dimensions(dimensions) { }

	virtual Item Clone(int count)
	{
		return Item(baseClass, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item Clone()
	{
		return Item(baseClass, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item* Clone2(int count)
	{
		return new Item(baseClass, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item* Clone2()
	{
		return new Item(baseClass, name, typeName, color, damage, count, range, dimensions);
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

	virtual void OnDeath(Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType);
};
Item* dItem = new Item("NULL", "NULL TYPE", olc::MAGENTA, 0, 0);

class GoneOnLandItem : public Item
{
public:
	using Item::Item;

	void OnDeath(Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType) override { }

	virtual Item Clone(int count)
	{
		return GoneOnLandItem(baseClass, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item Clone()
	{
		return GoneOnLandItem(baseClass, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item* Clone2(int count)
	{
		return new GoneOnLandItem(baseClass, name, typeName, color, damage, count, range, dimensions);
	}

	virtual Item* Clone2()
	{
		return new GoneOnLandItem(baseClass, name, typeName, color, damage, count, range, dimensions);
	}
};

typedef vector<Item> Cost;

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
						currentIndex--;
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
		if (index >= size())
			return false;
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

	bool TryTakeIndex(int index, Item& result)
	{
		if (index >= size())
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

	bool TryMake(Cost cost)
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
		for (int i = 0; i < size(); i++)
		{
			game->FillRect(Vec2(0, game->ScreenHeight() - 7 * (i + 1)), Vec2(7, 7), (*this)[i].color);
			game->DrawString(Vec2(3, game->ScreenHeight() - 7 * (i + 1)),
				"-" + to_string((*this)[i].count) + "-" + (i == currentIndex ? (*this)[i].name : (*this)[i].typeName), (*this)[i].color);
		}
		if (size() > 0)
			game->DrawRect(Vec2(0, game->ScreenHeight() - 7 * (currentIndex + 1)), Vec2(6, 6), olc::BLACK);
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
	Item* copper = new Item("Copper", "Ammo", Color(232, 107, 5), 1);
	GoneOnLandItem* iron = new GoneOnLandItem("Iron", "Ammo", Color(111, 123, 128), 12);
	Item* rock = new Item("Rock", "Ammo", Color(145, 141, 118), 6, 1, 5.0f);
}

#pragma endregion

namespace Costs
{
	Cost dRecipe{};
	Cost basicBullet{ Resources::copper->Clone(3) };
	Cost copperWall{ Resources::copper->Clone(9) };
	Cost duct{ Resources::copper->Clone(9), Resources::iron->Clone() };
	Cost smallVacuum{ Resources::copper->Clone(3), Resources::iron->Clone(5) };
	Cost largeVacuum{ Resources::copper->Clone(300), Resources::iron->Clone(50) };
	Cost turret{ Resources::copper->Clone() };//Resources::copper->Clone(100), Resources::iron->Clone(10) };
};