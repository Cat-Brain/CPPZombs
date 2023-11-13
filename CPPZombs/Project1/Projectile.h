#include "Entities.h"

namespace Updates { void ProjectileU(Entity* entity); } namespace VUpdates { void ProjectileVU(Entity* entity); }
EntityData projectileData = EntityData(Updates::ProjectileU, VUpdates::ProjectileVU, DUpdates::CollectibleDU);
class Projectile : public Entity
{
public:
    float range; // How far it can travel before landing.
    int damage; // It's damage on hit.
    float speed; // It's speed.
    float begin; // When it was created.
    HitResult hitType = HitResult::MISSED; // Internal value for what type of death this projectile had.
    bool shouldCollide; // Should this collide with objects.
    bool collideTerrain;
    VUpdate vUpdate; // Runs instead of described by data.
    ProjODFlags flags = 0;

    Projectile(EntityData* data, float range = 10, int damage = 1, float speed = 8.0f, float radius = 0.5f, VUpdate vUpdate = VUpdates::EntityVU, RGBA color = RGBA(),
        float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, Allegiance allegiance = 0) :
        Entity(data, vZero, radius, color, mass, bounciness, maxHealth, health, name, allegiance),
        range(range), damage(damage), speed(speed), vUpdate(vUpdate), begin(tTime), shouldCollide(shouldCollide), collideTerrain(collideTerrain)
    {
        sortLayer = 1;
        this->corporeal = corporeal;
        colOverlapFun = MaskF::IsCorporealNotCreatorNorSameType;
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
        HitResult result = HitResult::MISSED;
        if (projectile->collideTerrain)
        {
            if ((result = game->entities->TryDealDamageTiles(projectile->damage, projectile->pos, projectile->radius, MaskF::IsNonAlly, projectile)) != HitResult::MISSED)
                goto DestroySelf;
            Vec3 oldPos = projectile->pos;
            projectile->MovePos();
            if (oldPos != projectile->pos && (result = game->entities->TryDealDamageTiles(projectile->damage, projectile->pos, projectile->radius, MaskF::IsNonAlly, projectile)) != HitResult::MISSED)
                goto DestroySelf;
        }
        else
        {
            if ((result = game->entities->TryDealDamage(projectile->damage, projectile->pos, projectile->radius, MaskF::IsNonAlly, projectile)) != HitResult::MISSED)
                goto DestroySelf;
            Vec3 oldPos = projectile->pos;
            projectile->MovePos();
            if (oldPos != projectile->pos && (result = game->entities->TryDealDamage(projectile->damage, projectile->pos, projectile->radius, MaskF::IsNonAlly, projectile)) != HitResult::MISSED)
                goto DestroySelf;
        }
        return;
        DestroySelf:
        projectile->hitType = result;
        projectile->DestroySelf(nullptr);
    }
}

namespace VUpdates
{
    void ProjectileVU(Entity* entity)
    {
        static_cast<Projectile*>(entity)->vUpdate(entity);
    }
}

namespace OnDeaths { void ShotItemOD(Entity* entity, Entity* damageDealer); }
EntityData shotItemData = EntityData(Updates::ProjectileU, VUpdates::ProjectileVU, DUpdates::CollectibleDU, UIUpdates::EntityUIU, OnDeaths::ShotItemOD);
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

    ShotItem(ShotItem* baseClass, ItemInstance item, Vec3 pos, Vec3 direction, Entity* creator, ProjODFlags flags = 0) :
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
        maxHealth = item->health;
        name = item->name;
        if (creator != nullptr)
        {
            allegiance = creator->allegiance;
            creatorName = creator->name;
        }
        this->flags = flags;
        Start();
    }

    unique_ptr<Entity> Clone(ItemInstance baseItem, Vec3 pos, Vec3 direction, Entity* creator, ProjODFlags flags = 0)
    {
        return make_unique<ShotItem>(this, baseItem, pos, direction, creator, flags);
    }

    unique_ptr<Entity> Clone(Vec3 pos, Vec3 direction, Entity* creator) override
    {
        return Clone(item, pos, direction, creator);
    }
};

namespace OnDeaths
{
    void ShotItemOD(Entity* entity, Entity* damageDealer)
    {
        ShotItem* shot = static_cast<ShotItem*>(entity);
        shot->item->itemOD(shot->item, ProjODData(shot->pos, shot->dir, shot->vel, shot->creator, shot->creatorName, damageDealer, shot->hitType, shot->flags));
    }
}

ShotItem* basicShotItem = new ShotItem(&shotItemData, Resources::copper.Clone(), "Basic Shot Item");

namespace ItemUs
{
    void DefaultU(ItemInstance& item, ProjUData data)
    {
        game->entities->push_back(basicShotItem->Clone(item.Clone(1),
            data.pos, data.dir, data.creator));
        item.count--;
    }
}