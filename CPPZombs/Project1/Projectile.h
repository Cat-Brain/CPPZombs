#include "Entities.h"

class Projectile : public Entity
{
public:
    Vec2 direction;
    Vec2 offset; // Should rarely be outside of the range (0, 0) to (1, 1), whenever it exceeds subtract from it and add to pos.
    float duration;
    int damage;
    float speed, begin;
    int callType = 0;

    Projectile(float duration = 10, int damage = 1, float speed = 8.0f, iVec2 dimensions = iVec2(1, 1), RGBA color = RGBA(),
        RGBA subScat = RGBA(), float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME", bool corporeal = false) :
        Entity(iVec2(0, 0), dimensions, color, subScat, mass, maxHealth, health, name),
        duration(duration), damage(damage), speed(speed), begin(tTime)
    {
        update = UPDATE::PROJECTILEU;
        isProjectile = true;
        sortLayer = 1;
        this->corporeal = corporeal;
        Start();
    }

    Projectile(Projectile* baseClass, iVec2 pos, iVec2 direction, Entity* creator) :
        Projectile(*baseClass)
    {
        this->creator = creator;
        this->direction = glm::normalize(Vec2(direction));
        offset = vZero;
        this->pos = pos;
        begin = tTime;
        Start();
    }

    unique_ptr<Entity> Clone(iVec2 pos, iVec2 direction, Entity* creator) override
    {
        return make_unique<Projectile>(this, pos, direction, creator);
    }

    bool CheckPos(Entity*& hitEntity)
    {
        vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(pos, dimensions);

        for (Entity* entity : hitEntities)
        {
            if (entity == creator || entity == this)
                continue;

            hitEntity = entity;
            if (entity->DealDamage(damage, this) == 1)
                hitEntity = nullptr;
            return true;
        }
        return false;
    }

    virtual void MovePos()
    {
        offset += direction * game->dTime * speed;
        if (iVec2(offset) != vZero)
        {
            TryMove(iVec2(offset), mass + mass, creator);
            offset -= iVec2(offset);
        }
    }
};

namespace Updates
{
    void ProjectileU(Entity* _entity)
    {
        Projectile* projectile = static_cast<Projectile*>(_entity);

        if (tTime - projectile->begin >= (projectile->duration + 1) / projectile->speed)
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
        iVec2 oldPos = projectile->pos;
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

    ShotItem(Item item, float speed = 8.0f, Vec2 dimensions = Vec2(1, 1), float mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(item.range, item.damage, speed, dimensions, item.color, RGBA(), mass, maxHealth, health), item(item)
    {
        onDeath = ONDEATH::SHOTITEMOD;
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec2 pos, Vec2 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->creator = creator;
        float magnitude = glm::length(direction);
        this->direction = direction / magnitude;
        duration = fminf(item.range, magnitude);
        offset = vZero;
        this->pos = pos;
        begin = tTime;
        damage = item.damage;
        creatorName = creator->name;
        health = item.health;
        Start();
    }

    ShotItem(ShotItem* baseClass, Item item, Vec2 pos, Vec2 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->creator = creator;
        float magnitude = glm::length(direction);
        this->direction = direction / magnitude;
        duration = fminf(item.range, magnitude);
        offset = vZero;
        this->pos = pos;
        begin = tTime;
        damage = item.damage;
        this->item = item;
        color = item.color;
        subScat = item.subScat;
        mass = item.mass;
        corporeal = item.corporeal;
        dimensions = item.dimensions;
        health = item.health;
            name = item.name;
        if (creator != nullptr)
            creatorName = creator->name;
        Start();
    }

    unique_ptr<Entity> Clone(iVec2 pos, iVec2 direction, Entity* creator) override
    {
        return make_unique<ShotItem>(this, pos, direction, creator);
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

ShotItem* basicShotItem = new ShotItem(*Resources::copper, 12, vOne, 1, 1, 1);