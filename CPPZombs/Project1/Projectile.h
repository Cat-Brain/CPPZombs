#include "Entities.h"

class Projectile : public Entity
{
public:
    float duration;
    int damage;
    float speed, begin;
    int callType = 0;
    bool shouldCollide;

    Projectile(float duration = 10, int damage = 1, float speed = 8.0f, float radius = 0.5f, RGBA color = RGBA(),
        float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME", bool corporeal = false, bool shouldCollide = true, Allegiance allegiance = 0) :
        Entity(vZero, radius, color, mass, maxHealth, health, name, allegiance),
        duration(duration), damage(damage), speed(speed), begin(tTime), shouldCollide(shouldCollide)
    {
        update = UPDATE::PROJECTILE;
        isProjectile = true;
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

class ShotItem : public Projectile
{
public:
    ItemInstance item;
    string creatorName;

    ShotItem(ItemInstance item, float speed = 8.0f, float radius = 0.5f, float mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(item->range, item->damage, speed, radius, item->color, mass, maxHealth, health), item(item)
    {
        onDeath = ONDEATH::SHOTITEM;
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
};

namespace OnDeaths
{
    void ShotItemOD(Entity* entity, Entity* damageDealer)
    {
        ShotItem* shot = static_cast<ShotItem*>(entity);
        shot->item->OnDeath(shot->item, shot->pos, shot->dir, shot->creator, shot->creatorName, damageDealer, shot->callType);
    }
}

ShotItem* basicShotItem = new ShotItem(Resources::copper.Clone(), 12, 0.5f, 1, 1, 1);

namespace ItemUs
{
    void ItemU(ItemInstance& item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
    {
        game->entities->push_back(basicShotItem->Clone(item.Clone(1),
            pos + Vec3(0, 0, 0.1f + item->radius - creator->radius), dir, creator));
        item.count--;
    }
}