#include "Include.h"

class Item
{
public:
	Item* baseClass;
	string name;
	Color color;
	int damage;
	int count;
	float range;

	Item(string name = "NULL", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f) :
		baseClass(this), name(name), color(color), damage(damage), count(count), range(range) { }

	Item(Item* baseClass, string name = "NULL", Color color = olc::MAGENTA, int damage = 1, int count = 1, float range = 15.0f) :
		baseClass(baseClass), name(name), color(color), damage(damage), count(count), range(range) { }

	virtual Item Clone(int count)
	{
		return Item(baseClass, name, color, damage, count, range);
	}

	virtual Item Clone()
	{
		return Item(baseClass, name, color, damage, count, range);
	}

	virtual Item* Clone2(int count)
	{
		return new Item(baseClass, name, color, damage, count, range);
	}

	virtual Item* Clone2()
	{
		return new Item(baseClass, name, color, damage, count, range);
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

	virtual void OnDeath(vector<void*>* collectibles, vector<void*>* entities, Vec2 pos);
};
Item* dItem = new Item("NULL", olc::MAGENTA, 0, 0);

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

	void DUpdate(Screen* screen)
	{
		for (int i = 0; i < size(); i++)
		{
			screen->FillRect(Vec2(0, screen->ScreenHeight() - 7 * (i + 1)), Vec2(7, 7), (*this)[i].color);
			screen->DrawString(Vec2(3, screen->ScreenHeight() - 7 * (i + 1)), "-" + to_string((*this)[i].count), (*this)[i].color);
		}
		if (size() > 0)
			screen->DrawRect(Vec2(0, screen->ScreenHeight() - 7 * (currentIndex + 1)), Vec2(6, 6), olc::BLACK);
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
	Item* copper = new Item("Copper", Color(232, 107, 5), 1);
	Item* iron = new Item("Iron", Color(111, 123, 128), 3);
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
	Cost turret{ Resources::copper->Clone(100), Resources::iron->Clone(10) };
};