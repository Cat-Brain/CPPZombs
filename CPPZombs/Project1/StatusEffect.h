#include "Planet.h"

class Infliction
{
public:
	uint functionIndex;
	float secPerTick;
	string name;

	Infliction(uint functionIndex, float secPerTick, string name):
		functionIndex(functionIndex), secPerTick(secPerTick) {}
};

class StatusEffect;
bool FireFunc(StatusEffect* status), BleedFunc(StatusEffect* status), WetFunc(StatusEffect* status);
vector<function<bool(StatusEffect* status)>> inflictionFuncs;

vector<Infliction> inflictions = {Infliction(0, 0.2f, "Fire"), Infliction(0, 0.2f, "Bleed") , Infliction(0, 0.2f, "Wet") };

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

	StatusEffect(STATUS status, float length) :
		status(status), timeTillTick(inflictions[UnEnum(status)].secPerTick), timeTillRemove(length) {}

	void Call()
	{
		inflictionFuncs[inflictions[UnEnum(status)].functionIndex](this);
	}

	void Update()
	{
		timeTillTick -= game->dTime;
		if (timeTillTick < 0)
		{
			timeTillTick += inflictions[UnEnum(status)].secPerTick;
			Call();
		}
		timeTillRemove -= game->dTime;
		if (timeTillRemove < 0)
			;
	}

	bool FireFunc()
	{

	}
};

class Hit
{

};