#include "Planet.h"

class Infliction
{
public:
	uint start, update, end;
	float secPerTick;
	string name;

	Infliction(uint start, uint update, uint end, float secPerTick, string name):
		start(start), update(update), end(end), secPerTick(secPerTick) {}
};
class StatusEffect;
namespace StatusFuncs
{
	typedef void StatFunc(StatusEffect& status);

	StatFunc NoStr, WetStr;
	StatFunc NoUpd, FireUpd, BleedUpd, WetUpd;
	StatFunc NoEnd, WetEnd;

	vector<function<void(StatusEffect& status)>> inflictionStarts{ NoStr, WetStr };
	vector<function<void(StatusEffect& status)>> inflictionUpdates{ NoUpd, FireUpd, BleedUpd, WetUpd };
	vector<function<void(StatusEffect& status)>> inflictionEnds{ NoEnd, WetEnd };
}

vector<Infliction> inflictions = {Infliction(0, 1, 0, 0.2f, "Fire"), Infliction(0, 2, 0, 0.2f, "Bleed"), Infliction(1, 3, 1, 0.2f, "Wet") };

enum class STATUS
{
	FIRE, BLEED, WET
};

class StatusEffect
{
public:
	Entity* entity;
	STATUS status;
	float timeTillTick;
	float timeTillRemove;

	StatusEffect(Entity* entity, STATUS status, float length) :
		entity(entity), status(status), timeTillTick(Type()->secPerTick), timeTillRemove(length)
	{
		Str();
	}


	inline Infliction* Type()
	{
		return &inflictions[UnEnum(status)];
	}

	inline Infliction* operator->()
	{
		return Type();
	}

protected:
	void Str()
	{
		StatusFuncs::inflictionStarts[Type()->start](*this);
	}

	void Upd()
	{
		StatusFuncs::inflictionUpdates[Type()->update](*this);
	}

	void End()
	{
		StatusFuncs::inflictionEnds[Type()->end](*this);
	}
public:

	bool Update()
	{
		timeTillTick -= game->dTime;
		if (timeTillTick < 0)
		{
			timeTillTick += Type()->secPerTick;
			Upd();
		}
		timeTillRemove -= game->dTime;
		if (timeTillRemove >= 0) // Is it alive?
			return false; // Yes so don't destroy.
		// Else should be destroyed:
		End();
		return true; // Yes destroy this now.
	}
};

class Hit
{

};