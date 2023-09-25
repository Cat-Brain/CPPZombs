#include "Entities.h"

EntityData projectileData = EntityData(UPDATE::PROJECTILE, VUPDATE::PROJECTILE, DUPDATE::PROJECTILE);
class Projectile : public Entity
{
public:
    float range; // How far it can travel before landing.
    int damage; // It's damage on hit.
    float speed; // It's speed.
    float begin; // When it was created.
    int callType = 0; // Internal value for what type of death this projectile had.
    bool shouldCollide; // Should this collide with objects.
    bool collideTerrain;
    VUPDATE vUpdate; // Runs instead of described by data.

    Projectile(EntityData* data, float range = 10, int damage = 1, float speed = 8.0f, float radius = 0.5f, VUPDATE vUpdate = VUPDATE::ENTITY, RGBA color = RGBA(),
        float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, Allegiance allegiance = 0) :
        Entity(data, vZero, radius, color, mass, bounciness, maxHealth, health, name, allegiance),
        range(range), damage(damage), speed(speed), vUpdate(vUpdate), begin(tTime), shouldCollide(shouldCollide), collideTerrain(collideTerrain)
    {
        sortLayer = 1;
        this->corporeal = corporeal;
        Start();
    }

    Projectile(Projectile* baseClass, Vec3 pos, Vec3 dir, Entity* creator) :
        Projectile(*baseClass)
    {
        if (creator != nullptr)
        {
            this->creator = creator;
            allegiance = creator->allegiance;
        }
        this->dir = Normalized(dir);
        this->pos = pos;
        begin = tTime;
        Start();
    }

    unique_ptr<Entity> Clone(Vec3 pos, Vec3 direction, Entity* creator) override
    {
        return make_unique<Projectile>(this, pos, direction, creator);
    }

    virtual void MovePos()
    {
        SetPos(pos + dir * game->dTime * speed);
    }
};

namespace Updates
{
    void ProjectileU(Entity* _entity)
    {
        Projectile* projectile = static_cast<Projectile*>(_entity);

        if (tTime - projectile->begin >= projectile->range / projectile->speed)
            return projectile->DestroySelf(projectile);

        if (!projectile->shouldCollide)
            return projectile->MovePos();

        int result;
        if (projectile->collideTerrain)
        {
            if ((result = game->entities->TryDealDamageTiles(projectile->damage, projectile->pos, projectile->radius, MaskF::IsNonAlly, projectile)) != 0)
            {
                projectile->callType = 1 + int(result == 3);
                projectile->DestroySelf(nullptr);
                return;
            }
            Vec3 oldPos = projectile->pos;
            projectile->MovePos();
            if (oldPos != projectile->pos && (result = game->entities->TryDealDamageTiles(projectile->damage, projectile->pos, projectile->radius, MaskF::IsNonAlly, projectile)) != 0)
            {
                projectile->callType = 1 + int(result == 3);
                projectile->DestroySelf(nullptr);
            }
        }
        else
        {
            if ((result = game->entities->TryDealDamage(projectile->damage, projectile->pos, projectile->radius, MaskF::IsNonAlly, projectile)) != 0)
            {
                projectile->callType = 1 + int(result == 3);
                projectile->DestroySelf(nullptr);
                return;
            }
            Vec3 oldPos = projectile->pos;
            projectile->MovePos();
            if (oldPos != projectile->pos && (result = game->entities->TryDealDamage(projectile->damage, projectile->pos, projectile->radius, MaskF::IsNonAlly, projectile)) != 0)
            {
                projectile->callType = 1 + int(result == 3);
                projectile->DestroySelf(nullptr);
            }
        }
    }
}

namespace VUpdates
{
    void ProjectileVU(Entity* entity)
    {
        entity->VUpdate(static_cast<Projectile*>(entity)->vUpdate);
    }
}

namespace DUpdates
{
    void ProjectileDU(Entity* entity)
    {
        if (glm::distance2(game->PlayerPos(), entity->pos) < entity->radius * entity->radius + game->playerE->radius * game->playerE->radius +
            COLLECTIBLE_TRANSPARENT_DIST * COLLECTIBLE_TRANSPARENT_DIST)
        {
            byte alpha = entity->color.a;
            entity->color.a = static_cast<byte>(entity->color.a * (glm::distance(game->PlayerPos(), entity->pos) /
                (COLLECTIBLE_TRANSPARENT_DIST + entity->radius + game->playerE->radius)));
            entity->DUpdate(DUPDATE::ENTITY);
            entity->color.a = alpha;
            return;
        }
        entity->DUpdate(DUPDATE::ENTITY);
    }
}

EntityData shotItemData = EntityData(UPDATE::PROJECTILE, VUPDATE::PROJECTILE, DUPDATE::PROJECTILE, UIUPDATE::ENTITY, ONDEATH::SHOTITEM);
class ShotItem : public Projectile
{
public:
    ItemInstance item;
    string creatorName;

    ShotItem(EntityData* data, ItemInstance item, string name) :
        Projectile(data, item->range, item->damage, item->speed, item->radius, item->vUpdate, item->color, item->mass, 0, item->health, item->health, name, item->corporeal, item->shouldCollide, item->collideTerrain), item(item)
    {
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec3 pos, Vec3 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->creator = creator;
        this->dir = Normalized(direction);
        range = item->range;
        this->pos = pos;
        begin = tTime;
        if (creator != nullptr)
        {
            allegiance = creator->allegiance;
            creatorName = creator->name;
        }
        Start();
    }

    ShotItem(ShotItem* baseClass, ItemInstance item, Vec3 pos, Vec3 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        vUpdate = item->vUpdate;
        this->creator = creator;
        this->dir = Normalized(direction);
        range = item->range;
        this->pos = pos;
        begin = tTime;
        damage = item->damage;
        this->item = item;
        color = item->color;
        mass = item->mass;
        speed = item->speed;
        corporeal = item->corporeal;
        shouldCollide = item->shouldCollide;
        collideTerrain = item->collideTerrain;
        radius = item->radius;
        health = item->health;
        name = item->name;
        if (creator != nullptr)
        {
            allegiance = creator->allegiance;
            creatorName = creator->name;
        }
        Start();
    }

    unique_ptr<Entity> Clone(ItemInstance baseItem, Vec3 pos, Vec3 direction, Entity* creator)
    {
        return make_unique<ShotItem>(this, baseItem, pos, direction, creator);
    }

    unique_ptr<Entity> Clone(Vec3 pos, Vec3 direction, Entity* creator) override
    {
        return make_unique<ShotItem>(this, pos, direction, creator);
    }
};

namespace OnDeaths
{
    void ShotItemOD(Entity* entity, Entity* damageDealer)
    {
        ShotItem* shot = static_cast<ShotItem*>(entity);
        shot->item->OnDeath(shot->item, shot->pos, shot->dir, shot->vel, shot->creator, shot->creatorName, damageDealer, shot->callType);
    }
}

ShotItem* basicShotItem = new ShotItem(&shotItemData, Resources::copper.Clone(), "Basic Shot Item");

namespace ItemUs
{
    void ItemU(ItemInstance& item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
    {
        game->entities->push_back(basicShotItem->Clone(item.Clone(1),
            pos, dir, creator));
        item.count--;
    }
}