#include "Item.h"

#define COMMON_TEXT_SCALE trueScreenHeight / 20
#define COMMON_BOARDER_WIDTH trueScreenHeight / 80

class TimedEvent
{
public:
	float startTime;
	std::function<bool(void* entity, TimedEvent* mEvent)> function;

	TimedEvent(std::function<bool(void* entity, TimedEvent* mEvent)> function) :
		function(function), startTime(tTime) { }
};

enum ALLEGIANCES : byte
{
	BARBARIAN_A = 0, PLAYER_A = 1, WILD_A = 2, COMPANY_A = 4
};
struct Allegiance
{
	bool a : 1; //   1 - Player
	bool b : 1; //   2 - Wild
	bool c : 1; //   4 - Company
	bool d : 1; //   8 - 
	bool e : 1; //  16 - 
	bool f : 1; //  32 - 
	bool g : 1; //  64 - 
	bool h : 1; // 128 - 

	Allegiance(byte value = 0)
	{
		*this = value; // Uses custom = operator;
	}

	inline void operator = (byte value)
	{
		*(byte*)this = value;
	}

	inline Allegiance operator | (Allegiance other)
	{
		return *((byte*)this) | *((byte*)&other);
	}

	inline Allegiance operator + (Allegiance other)
	{
		return *this | other;
	}

	inline byte OverlapByte(Allegiance other)
	{
		return *((byte*)this) & *((byte*)&other);
	}

	inline Allegiance Overlap(Allegiance other)
	{
		return Allegiance(OverlapByte(other));
	}

	inline bool Ally(Allegiance other)
	{
		return OverlapByte(other) != 0;
	}
};

class Entities;
class EntityData;

class Entity
{
public:
	EntityData* data;
	Entity* baseClass;
	Entity* creator;
	vector<Entity*> observers{};
	vector<StatusEffect> statuses{};
	string name;
	Vec3 pos, lastPos, vel, dir;
	float radius;
	RGBA color;
	float mass, bounciness, frictionMultiplier = 1;
	int maxHealth, health;
	bool active = true, dActive = true, uiActive = false;
	int sortLayer = 0;
	float foodValue = 0; // If 0 then is not food.
	bool isCollectible = false, corporeal = true;
	Allegiance allegiance;
	EntityMaskFun colOverlapFun;

	Entity(EntityData* data, Vec3 pos = vZero, float radius = 0.5f, RGBA color = RGBA(),
		float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", Allegiance allegiance = BARBARIAN_A);

	Entity(Entity* baseClass, Vec3 pos):
		Entity(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	virtual unique_ptr<Entity> Clone(Vec3 pos = Vec3(0), Vec3 dir = Vec3(0, 1, 0), Entity* creator = nullptr)
	{
		return make_unique<Entity>(this, pos);
	}

	virtual void Start() { }

	void Draw(Vec3 pos);

	Vec2 BottomLeft() // Not always accurate.
	{
		return vZero;//(pos - game->PlayerPos() + Vec3(radius, -radius, -radius)) * (trueScreenHeight / game->zoom);
	}

	void DrawUIBox(Vec2 bottomLeft, Vec2 topRight, float boarderWidth, string text, RGBA textColor,
		RGBA borderColor = RGBA(127, 127, 127), RGBA fillColor = RGBA(63, 63, 63))
	{
		game->DrawFBL(bottomLeft, borderColor, topRight - bottomLeft + boarderWidth);
		game->DrawFBL(bottomLeft + boarderWidth, fillColor, topRight - bottomLeft); // Draw the middle box, +1.
		font.Render(text, bottomLeft + boarderWidth + south2 * (font.mininumVertOffset / 2.f), static_cast<float>(COMMON_TEXT_SCALE), textColor);
	}

	virtual void SetPos(Vec3 newPos);
	virtual void SetRadius(float newRadius);

	virtual HitResult ApplyHitHarmless(int damage, Entity* damageDealer); // 0 = lived, 1 = dead
	virtual HitResult ApplyHit(int damage, Entity* damageDealer) // 0 = lived, 1 = dead
	{
		HitResult result = ApplyHitHarmless(damage, damageDealer);
		if (result == HitResult::DIED)
			DestroySelf(damageDealer);
		return result;
	}

	void DestroySelf(Entity* damageDealer); // Always calls OnDeath;
	void DelayedDestroySelf(); // Never calls OnDeath;

	virtual bool Overlaps(Vec3 pos, float radius)
	{
		return glm::length2(this->pos - pos) < (this->radius + radius) * (this->radius + radius);
	}

	void UpdateChunkCollision();
	void UpdateCollision();

	virtual void UnAttach(Entity* entity) { }
	virtual void MoveAttachment(Entity* from, Entity* to) { }

	void DetachObservers()
	{
		for (Entity* entity : observers)
			entity->UnAttach(this);
		observers.clear();
	}

	void AddObserver(Entity* observer)
	{
		observers.push_back(observer);
	}

	void RemoveObserver(Entity* observer)
	{
		observers.erase(find(observers.begin(), observers.end(), observer));
	}
};



#pragma region Other Entity funcitons
namespace MaskF
{
	bool CanCollide(Entity* from, Entity* to)
	{
		return from->colOverlapFun(from, to) && to->colOverlapFun(to, from);
	}

	bool IsDifferent(Entity* from, Entity* to)
	{
		return from != to;
	}

	bool IsCollectible(Entity* from, Entity* to)
	{
		return from != to && to->isCollectible;
	}

	bool IsCorporeal(Entity* from, Entity* to)
	{
		return from != to && to->corporeal;
	}

	bool IsCorporealNotCollectible(Entity* from, Entity* to)
	{
		return from != to && to->corporeal && !to->isCollectible;
	}

	bool IsCorporealNotCreator(Entity* from, Entity* to)
	{
		return from != to && from->creator != to && to->corporeal && !to->isCollectible;
	}

	bool IsCorporealNotCreatorNorSameType(Entity* from, Entity* to)
	{
		return from != to && from->creator != to && to->corporeal && !to->isCollectible && from->baseClass != to->baseClass;
	}

	bool IsNonAlly(Entity* from, Entity* to)
	{
		return from != to && to->corporeal && !to->isCollectible && !from->allegiance.Ally(to->allegiance);
	}

	bool IsNonAllyOrCreator(Entity* from, Entity* to)
	{
		return from != to && to->corporeal && !to->isCollectible && (!from->allegiance.Ally(to->allegiance) || from->creator == to);
	}

	bool IsAlly(Entity* from, Entity* to)
	{
		return from != to && to->corporeal && !to->isCollectible && from->allegiance.Ally(to->allegiance);
	}

	bool IsSameType(Entity* from, Entity* to)
	{
		return from->baseClass == to->baseClass && from != to && to->corporeal;
	}
}
namespace ExtrF
{
	float SqrDist(Entity* from, Entity* to)
	{
		return glm::distance2(from->pos, to->pos);
	}

	float DistMin(Entity* from, Entity* to)
	{
		return glm::distance(from->pos, to->pos) - from->radius - to->radius;
	}
}

Entity::Entity(EntityData* data, Vec3 pos, float radius, RGBA color,
	float mass, float bounciness, int maxHealth, int health, string name, Allegiance allegiance) :
	data(data), pos(pos), lastPos(pos), vel(0), dir(0), radius(radius), color(color), allegiance(allegiance),
	mass(mass), bounciness(bounciness), maxHealth(maxHealth), health(health), name(name), baseClass(this), creator(nullptr),
	colOverlapFun(MaskF::IsCorporealNotCollectible)
{ }

bool BoxCircleOverlap(Vec3 cPos, float radius, Vec3 bPos, Vec3 bDim)
{
	Vec3 difference = cPos - bPos;
	Vec3 clamped = glm::clamp(difference, -bDim, bDim);
	// add clamped value to AABB_center and we get the value of box closest to circle
	Vec3 closest = bPos + clamped;
	// retrieve vector between center circle and closest point AABB and check if length <= radius
	difference = closest - cPos;
	return glm::length2(difference) < radius * radius;
}

vector<Entity*> EntitiesOverlaps(Vec3 pos, float radius, vector<Entity*> entities)
{
	vector<Entity*> foundEntities(0);
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->Overlaps(pos, radius))
			foundEntities.push_back(*i);
	return foundEntities;
}

#pragma endregion


class FadeOut : public Entity
{
public:
	float startTime, totalFadeTime;

	FadeOut(EntityData* data, float totalFadeTime = 1.0f, Vec3 pos = Vec3(0), float radius = 0.5f, RGBA color = RGBA()) :
		Entity(data, pos, radius, color), totalFadeTime(totalFadeTime), startTime(tTime)
	{
		corporeal = false;
	}

	float Opacity()
	{
		return 0.1f * roundf((1.0f - (tTime - startTime) / totalFadeTime) * 10);
	}
};

namespace Updates
{
	void EntityU(Entity* entity) { }

	void FadeOutU(Entity* entity)
	{
		if (tTime - ((FadeOut*)entity)->startTime > ((FadeOut*)entity)->totalFadeTime)
			entity->DestroySelf(entity);
	}
}

namespace VUpdates
{
	void EntityVU(Entity* entity)
	{
		entity->lastPos = entity->pos;
		entity->SetPos(entity->pos + entity->vel * game->dTime);
	}

	void AirResistanceVU(Entity* entity)
	{
		entity->lastPos = entity->pos;
		entity->SetPos(entity->pos + entity->vel * game->dTime);
		entity->vel = FromTo(entity->vel, vZero, game->dTime * game->planet->friction * entity->frictionMultiplier);
	}

	void GravityVU(Entity* entity)
	{
		entity->lastPos = entity->pos;
		entity->SetPos(entity->pos + entity->vel * game->dTime);
		entity->vel = Vec3((Vec2)entity->vel, (entity->vel.z - game->planet->gravity * game->dTime) * powf(0.75f, game->dTime));
	}

	void GravityTrueVU(Entity* entity)
	{
		entity->lastPos = entity->pos;
		entity->SetPos(entity->pos + entity->vel * game->dTime);
		entity->vel.z -= game->planet->gravity * game->dTime;
	}

	void FrictionVU(Entity* entity)
	{
		entity->lastPos = entity->pos;
		entity->SetPos(entity->pos + entity->vel * game->dTime);
		entity->vel = Vec3(FromTo2(entity->vel, vZero, game->dTime * game->planet->friction * entity->frictionMultiplier),
			(entity->vel.z - game->planet->gravity * game->dTime) * powf(0.75f, game->dTime));
	}
}

namespace DUpdates
{
	void EntityDU(Entity* entity) // Normally only draws.
	{
		game->DrawCircle(entity->pos, entity->color, entity->radius);
	}

	void FadeOutDU(Entity* entity)
	{
		FadeOut* fade = static_cast<FadeOut*>(entity);
		uint a = fade->color.a;
		fade->color.a = static_cast<byte>((a / 255.f) * (255 - static_cast<uint8_t>((tTime - fade->startTime) * 255 / fade->totalFadeTime)));
		EntityDU(entity);
		fade->color.a = a;
	}
}

namespace EDUpdates { void EntityEDU(Entity* entity) { } }

namespace UIUpdates
{
	void EntityUIU(Entity* entity)
	{
		Vec2 bottomLeft = entity->BottomLeft();
		Vec2 topRight = bottomLeft + Vec2(font.TextWidth(entity->name) * COMMON_TEXT_SCALE / font.minimumSize, font.maxVertOffset / 2) * 0.5f;
		entity->DrawUIBox(bottomLeft, topRight, static_cast<float>(COMMON_BOARDER_WIDTH), entity->name, entity->color);
	}
}

namespace OnDeaths { void EntityOD(Entity* entity, Entity* damageDealer) { } }

namespace OverlapRes
{
	void CircleOR(Entity* a, Entity* b)
	{
		float dist = glm::length(b->pos - a->pos);
		if (dist <= 0) return;

		// Compute normal
		Vec3 v_un = (b->pos - a->pos) / dist;

		Vec3 tanA = Normalized(glm::cross(v_un, glm::cross(v_un, a->vel)));
		Vec3 tanB = Normalized(glm::cross(v_un, glm::cross(v_un, b->vel)));

		float v1t = glm::dot(tanA, a->vel);
		float v2t = glm::dot(tanB, b->vel);

		float v1n = glm::dot(v_un, a->vel);
		float v2n = glm::dot(v_un, b->vel);

		float m1 = (v1n * (a->mass - b->mass) + 2.0f * b->mass * v2n) / (a->mass + b->mass);
		float m2 = (v2n * (b->mass - a->mass) + 2.0f * a->mass * v1n) / (a->mass + b->mass);
		
		// Set new velocities in x and y coordinates
		a->vel = tanA * v1t + v_un * m1;
		b->vel = tanB * v2t + v_un * m2;

		Vec3 multiplier = (b->pos - a->pos) * (1.01f * (dist - a->radius - b->radius) / (dist * (a->mass + b->mass)));
		a->SetPos(a->pos + multiplier * b->mass);
		b->SetPos(b->pos - multiplier * a->mass);
	}
}

class EntityData
{
public:
	Update update;
	VUpdate vUpdate;
	DUpdate dUpdate;
	UIUpdate uiUpdate;
	OnDeath onDeath;

	EntityData(Update update = Updates::EntityU, VUpdate vUpdate = VUpdates::EntityVU, DUpdate dUpdate = DUpdates::EntityDU,
		UIUpdate uiUpdate = UIUpdates::EntityUIU, OnDeath onDeath = OnDeaths::EntityOD) :
		update(update), vUpdate(vUpdate), dUpdate(dUpdate),
		uiUpdate(uiUpdate), onDeath(onDeath) { }
};

void Entity::Draw(Vec3 pos)
{
	Vec3 tempPos = this->pos;
	this->pos = pos;
	data->dUpdate(this);
	this->pos = tempPos;
}

EntityData fadeOutData = EntityData(Updates::FadeOutU, VUpdates::EntityVU , DUpdates::FadeOutDU);