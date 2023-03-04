#include "Entities.h"

class Projectile : public Entity
{
public:
    Vec2 direction = vZero;
    float duration;
    int damage;
    float speed, begin;
    int callType = 0;

    Projectile(float duration = 10, int damage = 1, float speed = 8.0f, float radius = 0.5f, RGBA color = RGBA(),
        RGBA subScat = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME", bool corporeal = false) :
        Entity(vZero, radius, color, subScat, mass, maxHealth, health, name),
        duration(duration), damage(damage), speed(speed), begin(tTime)
    {
        update = UPDATE::PROJECTILEU;
        isProjectile = true;
        sortLayer = 1;
        this->corporeal = corporeal;
        Start();
    }

    Projectile(Projectile* baseClass, Vec2 pos, Vec2 direction, Entity* creator) :
        Projectile(*baseClass)
    {
        this->creator = creator;
        this->direction = Normalized(Vec2(direction));
        this->pos = pos;
        begin = tTime;
        Start();
    }

    unique_ptr<Entity> Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return make_unique<Projectile>(this, pos, direction, creator);
    }

    bool CheckPos(Entity*& hitEntity)
    {
        vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(pos, radius);

        for (Entity* entity : hitEntities)
        {
            if (entity == creator || entity == this)
                continue;

            hitEntity = entity;
            Vec2 hitPos = hitEntity->pos;
            if (entity->DealDamage(damage, this) == 1)
            {
                SetPos(hitPos);
                hitEntity = nullptr;
            }
            return true;
        }
        return false;
    }

    virtual void MovePos()
    {
        TryMove(direction * game->dTime * speed, mass + mass, creator);
    }
};

namespace Updates
{
    void ProjectileU(Entity* _entity)
    {
        Projectile* projectile = static_cast<Projectile*>(_entity);

        if (tTime - projectile->begin >= projectile->duration / projectile->speed)
            return projectile->DestroySelf(projectile);

        // Change to looking at other more custom variable that can be stored in item better.
        if (projectile->corporeal)
            return projectile->MovePos();

        Entity* entity;

        if (projectile->CheckPos(entity))
        {
            projectile->callType = 1 + int(entity == nullptr);
            projectile->DestroySelf(entity);
            return;
        }
        Vec2 oldPos = projectile->pos;
        projectile->MovePos();
        if (oldPos != projectile->pos && projectile->CheckPos(entity))
        {
            if (entity != nullptr)
                projectile->SetPos(oldPos);
            projectile->callType = 1 + int(entity == nullptr);
            projectile->DestroySelf(entity);
        }
    }
}

class ShotItem : public Projectile
{
public:
    Item item;
    string creatorName;

    ShotItem(Item item, float speed = 8.0f, float radius = 0.5f, float mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(item.range, item.damage, speed, radius, item.color, RGBA(), mass, maxHealth, health), item(item)
    {
        onDeath = ONDEATH::SHOTITEMOD;
        Start();
    }

    ShotItem(ShotItem* baseClass, Item item, Vec2 pos, Vec2 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->creator = creator;
        float magnitude = glm::length(direction);
        this->direction = direction / magnitude;
        duration = fminf(item.range, magnitude);
        this->pos = pos;
        begin = tTime;
        damage = item.damage;
        this->item = item;
        color = item.color;
        subScat = item.subScat;
        mass = item.mass;
        corporeal = item.corporeal;
        radius = item.radius;
        health = item.health;
            name = item.name;
        if (creator != nullptr)
            creatorName = creator->name;
        Start();
    }

    unique_ptr<Entity> Clone(Item baseItem, Vec2 pos, Vec2 direction, Entity* creator)
    {
        return make_unique<ShotItem>(this, baseItem, pos, direction, creator);
    }
};

namespace OnDeaths
{
    void ShotItemOD(Entity* entity, Entity* damageDealer)
    {
        ShotItem* shot = static_cast<ShotItem*>(entity);
        shot->item.baseClass->OnDeath(shot->pos, shot->creator, shot->creatorName, damageDealer, shot->callType);
    }
}

namespace Projectiles
{
    Item* basicBullet = new Item("Basic bullet", "Ammo", 1, RGBA(55, 55, 55), RGBA(), 2, 1, 30.0f);
}

ShotItem* basicShotItem = new ShotItem(*Resources::copper, 12, 0.5f, 1, 1, 1);