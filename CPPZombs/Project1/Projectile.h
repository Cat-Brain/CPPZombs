#include "LightSource.h"

class Projectile : public Entity
{
public:
    Vec2 direction;
    float duration;
    int damage;
    float lastTime, timePer, begin;
    int callType = 0; 

    Projectile(float duration = 10, int damage = 1, float timePer = 8.0f, Vec2f dimensions = Vec2f(1, 1), RGBA color = RGBA(),
        RGBA subsurfaceResistance = RGBA(), int mass = 1, int maxHealth = 1, int health = 1) :
        Entity(Vec2f(0, 0), dimensions, color, subsurfaceResistance, mass, maxHealth, health),
        duration(duration), damage(damage), timePer(timePer), begin(tTime)
    {
        Start();
    }

    Projectile(Projectile* baseClass, Vec2 pos, Vec2 direction, Entity* creator):
        Projectile(*baseClass)
    {
        this->creator = creator;
        this->direction = Vec2f(direction).Normalized();
        lastTime = 0.0f;
        this->pos = pos;
        begin = tTime;
        Start();
    }

    unique_ptr<Entity> Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return make_unique<Projectile>(this, pos, direction, creator);
    }

    void ReduceVel() override { }

    void Update() override
    {
        if(tTime - begin >= duration / speed)
            return DestroySelf(this);

        Entity* entity;

        if (CheckPos(entity))
        {
            callType = 1 + int(entity == nullptr);
            DestroySelf(entity);
            return;
        }
    }

    bool CheckPos(Entity*& hitEntity)
    {
        vector<Entity*> hitEntities = game->entities->FindCorpIOverlaps(iPos, dimensions);

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
    string creatorName;

    ShotItem(Item item, float speed = 8.0f, Vec2f dimensions = Vec2f(1, 1), int mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(item.range, item.damage, speed, dimensions, item.color, RGBA(), mass, maxHealth, health), item(item)
    {
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec2f pos, Vec2f direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        damage = item.damage;
        begin = tTime;
        this->creator = creator;
        this->pos = pos;
        float magnitude = direction.Magnitude();
        this->direction = direction / magnitude;
        vel = this->direction * speed;
        duration = fminf(item.range, magnitude);
        creatorName = creator->name;
        Start();
    }

    ShotItem(ShotItem* baseClass, Item item, Vec2f pos, Vec2f direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->item = item;
        damage = item.damage;
        color = item.color;
        begin = tTime;
        this->creator = creator;
        this->pos = pos;
        this->iPos = Vec2(pos);
        float magnitude = direction.Magnitude();
        this->direction = direction / magnitude;
        vel = this->direction * speed;
        duration = fminf(item.range, magnitude);
        this->dimensions = item.dimensions;
        if (creator == nullptr)
            name = item.name;
        else
        {
            vel += creator->vel;
            name = item.name + " shot by " + creator->name;
            creatorName = creator->name;
        }
        Start();
    }

    unique_ptr<Entity> Clone(Vec2f pos, Vec2f direction, Entity* creator) override
    {
        return make_unique<ShotItem>(this, pos, direction, creator);
    }

    unique_ptr<Entity> Clone(Item baseItem, Vec2f pos, Vec2f direction, Entity* creator)
    {
        return make_unique<ShotItem>(this, baseItem, pos, direction, creator);
    }

    void OnDeath(Entity* damageDealer) override
    {
        item.baseClass->OnDeath(iPos, creator, creatorName, damageDealer, callType);
        std::cout << iPos.x << ", " << iPos.y << " = " << pos.x << ", " << pos.y << "\n";
    }
};

namespace Projectiles
{
    Item* basicBullet = new Item("Basic bullet", "Ammo", RGBA(55, 55, 55), 2, 1, 30.0f);
}

ShotItem* basicShotItem = new ShotItem(*Resources::copper, 12, vOne, 1, 1, 1);