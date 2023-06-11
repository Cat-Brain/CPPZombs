#include "Entities.h"

EntityData projectileData = EntityData(UPDATE::PROJECTILE, VUPDATE::ENTITY, DUPDATE::PROJECTILE);
class Projectile : public Entity
{
public:
    float duration; // How far it can travel before landing.
    int damage; // It's damage on hit.
    float speed; // It's speed.
    float begin; // When it was created.
    int callType = 0; // Internal value for what type of death this projectile had.
    bool shouldCollide; // Should this collide with objects.
    bool collideTerrain;

    Projectile(EntityData* data, float duration = 10, int damage = 1, float speed = 8.0f, float radius = 0.5f, RGBA color = RGBA(),
        float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME", bool corporeal = false, bool shouldCollide = true, bool collideTerrain = false, Allegiance allegiance = 0) :
        Entity(data, vZero, radius, color, mass, bounciness, maxHealth, health, name, allegiance),
        duration(duration), damage(damage), speed(speed), begin(tTime), shouldCollide(shouldCollide), collideTerrain(collideTerrain)
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

        if (tTime - projectile->begin >= projectile->duration / projectile->speed)
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

namespace DUpdates
{
    void ProjectileDU(Entity* entity)
    {
        Projectile* projectile = static_cast<Projectile*>(entity);

        byte tempA = projectile->color.a;
        projectile->color.a = static_cast<byte>(255 * min(1.f, 2 * (tTime - projectile->begin)));
        projectile->DUpdate(DUPDATE::ENTITY);
        projectile->color.a = tempA;
    }
}

EntityData shotItemData = EntityData(UPDATE::PROJECTILE, VUPDATE::ENTITY, DUPDATE::PROJECTILE, EDUPDATE::ENTITY, UIUPDATE::ENTITY, ONDEATH::SHOTITEM);
class ShotItem : public Projectile
{
public:
    ItemInstance item;
    string creatorName;

    ShotItem(EntityData* data, ItemInstance item, string name) :
        Projectile(data, item->range, item->damage, item->speed, item->radius, item->color, item->mass, 0, item->health, item->health, name, item->corporeal, item->shouldCollide, item->collideTerrain), item(item)
    {
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec3 pos, Vec3 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->creator = creator;
        float magnitude = glm::length(direction);
        this->dir = direction / magnitude;
        duration = fminf(item->range, magnitude);
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
        this->creator = creator;
        float magnitude = glm::length(direction);
        this->dir = direction / magnitude;
        duration = fminf(item->range, magnitude);
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
        shot->item->OnDeath(shot->item, shot->pos, shot->dir, shot->creator, shot->creatorName, damageDealer, shot->callType);
    }
}

ShotItem* basicShotItem = new ShotItem(&shotItemData, Resources::copper.Clone(), "Basic Shot Item");

namespace ItemUs
{
    void ItemU(ItemInstance& item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
    {
        game->entities->push_back(basicShotItem->Clone(item.Clone(1),
            pos, dir, creator));
        /*game->entities->push_back(basicShotItem->Clone(item.Clone(1),
            pos + Vec3(0, 0, 0.1f + item->radius - creator->radius), dir, creator));*/
        item.count--;
    }
}