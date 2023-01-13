#include "Collectible.h"

class Projectile : public Entity
{
public:
    Vec2f direction;
    Vec2f fPos;
    float duration;
    int damage;
    float speed, begin;

    Projectile(float duration = 10, int damage = 1, float speed = 8.0f, Vec2 dimensions = Vec2(1, 1), Color color = olc::GREY, int mass = 1, int maxHealth = 1, int health = 1) :
        Entity(Vec2(0, 0), dimensions, color, mass, maxHealth, health),
        duration(duration), damage(damage), speed(speed), begin(tTime)
    {
        Start();
    }

    Projectile(Projectile* baseClass, Vec2f pos, Vec2f direction, Entity* creator):
        Projectile(*baseClass)
    {
        this->creator = creator;
        this->direction = Normalized(direction);
        fPos = pos + Vec2f(0.01f, 0.01f);
        this->pos = pos;
        begin = tTime;
        Start();
    }

    Entity* Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return new Projectile(this, pos, direction, creator);
    }

    void Update(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
    {
        if(tTime - begin >= duration / speed)
            return DestroySelf(entities, this);

        Entity* entity;

        if (CheckPos(entities, dTime, entity))
        {
            DestroySelf(entities, entity);
            return;
        }
        Vec2 oldPos = pos;
        MovePos(dTime);
        if (oldPos != pos && CheckPos(entities, dTime, entity))
        {
            pos = oldPos;
            DestroySelf(entities, entity);
        }
    }

    bool CheckPos(Entities* entities, float dTime, Entity*& hitEntity)
    {
        vector<Entity*> hitEntities = entities->FindCorpOverlaps(pos, dimensions);

        for (Entity* entity : hitEntities)
        {
            if (entity == creator || entity == this)
                return false;

            entity->DealDamage(damage, entities, this);
            hitEntity = entity;
            return true;
        }
        return false;
    }

    virtual void MovePos(float dTime)
    {
        fPos += direction * dTime * speed;
        pos = Vec2(static_cast<int>(roundf(fPos.x)), static_cast<int>(roundf(fPos.y)));
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


class ShotItem : public Projectile
{
public:
    Item item;

    ShotItem(Item item, float speed = 8.0f, Vec2 dimensions = Vec2(1, 1), int mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(item.range, item.damage, speed, dimensions, item.color, mass, maxHealth, health), item(item)
    {
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec2 pos, Vec2f direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        damage = item.damage;
        begin = tTime;
        this->creator = creator;
        fPos = pos + Vec2f(0.01f, 0.01f);
        float magnitude = Magnitude(direction);
        this->direction = direction / magnitude;
        duration = fminf(item.range, magnitude);
        this->pos = pos;
        Start();
    }

    ShotItem(ShotItem* baseClass, Item item, Vec2 pos, Vec2f direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->item = item;
        damage = item.damage;
        color = item.color;
        begin = tTime;
        this->creator = creator;
        fPos = pos + Vec2f(0.01f, 0.01f);
        float magnitude = Magnitude(direction);
        this->direction = direction / magnitude;
        duration = fminf(item.range, magnitude);
        this->pos = pos;
        name = item.name;
        Start();
    }

    Entity* Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return new ShotItem(this, pos, direction, creator);
    }

    Entity* Clone(Item baseItem, Vec2 pos, Vec2 direction, Entity* creator)
    {
        return new ShotItem(this, baseItem, pos, direction, creator);
    }

    void OnDeath(Entities* entities, Entity* damageDealer) override
    {
        item.baseClass->OnDeath(entities, pos, creator);
    }
};

namespace Projectiles
{
    Item* basicBullet = new Item("Basic bullet", "Ammo", olc::VERY_DARK_GREY, 2, 1, 30.0f);
}

ShotItem* basicShotItem = new ShotItem(*Resources::copper, 12, vOne, 1, 1, 1);