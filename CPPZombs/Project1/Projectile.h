#include "Entities.h"

class Projectile : public Entity
{
public:
    Vec2f direction;
    Vec2f fPos;
    float duration;
    int damage;
    float speed, begin;
    int callType = 0;

    Projectile(float duration = 10, int damage = 1, float speed = 8.0f, Vec2 dimensions = Vec2(1, 1), RGBA color = RGBA(),
        RGBA subScat = RGBA(), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
        Entity(Vec2(0, 0), dimensions, color, subScat, mass, maxHealth, health, name),
        duration(duration), damage(damage), speed(speed), begin(tTime)
    {
        update = UPDATE::PROJECTILE;
        Start();
    }

    Projectile(Projectile* baseClass, Vec2 pos, Vec2 direction, Entity* creator) :
        Projectile(*baseClass)
    {
        this->creator = creator;
        this->direction = Vec2f(direction).Normalized();
        fPos = pos + Vec2f(0.01f, 0.01f);
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
        fPos += direction * game->dTime * speed;
        SetPos(Vec2(static_cast<int>(roundf(fPos.x)), static_cast<int>(roundf(fPos.y))));
    }

    int SortOrder() override
    {
        return 1;
    }

    bool IsProjectile() override
    {
        return true;
    }

    bool Corporeal() override
    {
        return false;
    }
};

namespace Updates
{
    void Update(Entity* entity)
    {
        Projectile* projectile = static_cast<Projectile*>(entity);
        if (tTime - projectile->begin >= projectile->duration / projectile->speed)
            return projectile->DestroySelf(projectile);

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

    ShotItem(Item item, float speed = 8.0f, Vec2f dimensions = Vec2f(1, 1), int mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(item.range, item.damage, speed, dimensions, item.color, RGBA(), mass, maxHealth, health), item(item)
    {
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec2f pos, Vec2f direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->creator = creator;
        float magnitude = direction.Magnitude();
        this->direction = direction / magnitude;
        duration = fminf(item.range, magnitude);
        fPos = pos + Vec2f(0.01f, 0.01f);
        this->pos = pos;
        begin = tTime;
        damage = item.damage;
        creatorName = creator->name;
        Start();
    }

    ShotItem(ShotItem* baseClass, Item item, Vec2f pos, Vec2f direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->creator = creator;
        float magnitude = direction.Magnitude();
        this->direction = direction / magnitude;
        duration = fminf(item.range, magnitude);
        fPos = pos + Vec2f(0.01f, 0.01f);
        this->pos = pos;
        begin = tTime;
        damage = item.damage;
        this->item = item;
        color = item.color;
        subScat = item.subScat;
        dimensions = item.dimensions;
        if (creator == nullptr)
            name = item.name;
        else
            creatorName = creator->name;
        Start();
    }

    unique_ptr<Entity> Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return make_unique<ShotItem>(this, pos, direction, creator);
    }

    unique_ptr<Entity> Clone(Item baseItem, Vec2f pos, Vec2f direction, Entity* creator)
    {
        return make_unique<ShotItem>(this, baseItem, pos, direction, creator);
    }

    void OnDeath(Entity* damageDealer) override
    {
        item.baseClass->OnDeath(pos, creator, creatorName, damageDealer, callType);
    }
};

namespace Projectiles
{
    Item* basicBullet = new Item("Basic bullet", "Ammo", 1, RGBA(55, 55, 55), 2, 1, 30.0f);
}

ShotItem* basicShotItem = new ShotItem(*Resources::copper, 12, vOne, 1, 1, 1);