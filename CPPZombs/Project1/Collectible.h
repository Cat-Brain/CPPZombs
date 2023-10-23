#include "Particles.h"

namespace Updates { void CollectibleU(Entity* entity); } namespace DUpdates { void CollectibleDU(Entity* entity); }
EntityData collectibleData = EntityData(Updates::CollectibleU, VUpdates::FrictionVU, DUpdates::CollectibleDU);
class Collectible : public Entity
{
public:
	ItemInstance baseItem;

	Collectible(ItemInstance baseItem, Vec3 pos = vZero) :
		Entity(&collectibleData, pos, baseItem->radius, baseItem->color, 1, 0, 1, 1, baseItem->name), baseItem(baseItem)
	{
		isCollectible = true;
	}

	Collectible(ItemInstance baseItem, Vec3 pos, RGBA color) :
		Entity(&collectibleData, pos, baseItem->radius, color, 1, 0, 1, 1, baseItem->name), baseItem(baseItem)
	{
		corporeal = false;
		isCollectible = true;
		radius = FindRadius();
	}

	float FindRadius()
	{
		return cbrtf(baseItem->radius * baseItem->radius * baseItem->radius * baseItem.count);
	}

	Collectible(Collectible* baseClass, Vec3 pos) : Collectible(*baseClass) { this->pos = pos; }

	unique_ptr<Entity> Clone(Vec3 pos = Vec3(0), Vec3 dir = north, Entity* creator = nullptr) override
	{
		return make_unique<Collectible>(this, pos);
	}


	virtual Collectible* Clone(int count)
	{
		return new Collectible(baseItem.Clone(count), pos, color);
	}
};

namespace Collectibles
{
	Collectible* copper = new Collectible(Resources::copper.Clone());
	Collectible* iron = new Collectible(Resources::iron.Clone());
	Collectible* rock = new Collectible(Resources::rock.Clone());
	Collectible* bowler = new Collectible(Resources::bowler.Clone());
	Collectible* silver = new Collectible(Resources::silver.Clone());
}

#define DEFAULT_TRANSPARENCY_DISTANCE_LERP 5.f

float TransparencyDistanceLerp(Entity* a, Entity* b, float transparencyDistance = DEFAULT_TRANSPARENCY_DISTANCE_LERP)
{
	return (glm::distance2(a->pos, b->pos) < (transparencyDistance + a->radius + b->radius) *
		(transparencyDistance + a->radius + b->radius)) ? ClampF01((glm::distance(a->pos, b->pos) -
			a->radius - b->radius) / transparencyDistance) : 1;
}

namespace DUpdates
{
	void CollectibleDU(Entity* entity)
	{
		byte alpha = entity->color.a;
		entity->color.a = static_cast<byte>(entity->color.a * TransparencyDistanceLerp(entity, game->playerE));
		DUpdates::EntityDU(entity);
		entity->color.a = alpha;
	}
}