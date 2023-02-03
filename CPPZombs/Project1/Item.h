#include "Game1.h"

class Item
{
public:
	Item* baseClass;
	string name;
	string typeName;
	RGBA color;
	int damage;
	int count;
	float range;
	Vec2f dimensions;

	Item(string name = "NULL", string typeName = "NULL TYPE", RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, Vec2f dimensions = vOne) :
		baseClass(this), name(name), typeName(typeName), color(color), damage(damage), count(count), range(range), dimensions(dimensions) { }

	Item(Item* baseClass, string name = "NULL", string typeName = "NULL TYPE", RGBA color = RGBA(), int damage = 1, int count = 1, float range = 15.0f, Vec2f dimensions = vOne) :
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

	virtual void OnDeath(Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType);
};
Item* dItem = new Item("NULL", "NULL TYPE", RGBA(), 0, 0);

class GoneOnLandItem : public Item
{
public:
	using Item::Item;

	void OnDeath(Vec2f pos, Entity* creator, string creatorName, Entity* callReason, int callType) override { }

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
		if (size() == 0)
			return;
		int height = ScrHeight(), scale = height / (2 * max(8, int(size())));
		Vec2f offset = game->PlayerPos() - Vec2f(ScrDim()) * 0.5f;
		for (int i = 0; i < size(); i++)
		{
			if (i == currentIndex)
				continue;
			game->DrawFBL(offset + Vec2f(0, scale * i), (*this)[i].color, Vec2f(scale, scale));
			font.Render("-" + to_string((*this)[i].count) + "-" + (i == currentIndex ? (*this)[i].name : (*this)[i].typeName),
				Vec2(3, ScrHeight() - 7 * (i + 1)), 7.0f, (*this)[i].color);
		}
		game->DrawFBL(offset + Vec2f(0, scale * currentIndex), RGBA(), Vec2f(scale, scale));
		int scale2 = scale / 5;
		game->DrawFBL(offset + Vec2f(scale2, scale2 + scale * currentIndex), (*this)[currentIndex].color, Vec2f(scale, scale) - 2 * scale2);
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
	Item* copper = new Item("Copper", "Ammo", RGBA(232, 107, 5), 1);
	Item* iron = new Item("Iron", "Ammo", RGBA(111, 123, 128), 3);
	Item* rock = new Item("Rock", "Ammo", RGBA(145, 141, 118), 6, 1, 5.0f);
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