#include "Entity.h"

class Projectile : public Entity
{
public:
    Vec2f direction;
    Vec2f fPos;
    float duration;
    int damage;
    float speed, begin;

    Projectile(float duration = 10, int damage = 1, float speed = 8.0f, Color color = olc::GREY, int mass = 1, int maxHealth = 1, int health = 1) :
        Entity(Vec2(0, 0), color, mass, maxHealth, health),
        duration(duration), damage(damage), speed(speed), begin(tTime)
    {
        Start();
    }

    Projectile(Projectile* baseClass, Vec2 pos, Vec2f direction, Entity* creator):
        Projectile(*baseClass)
    {
        this->creator = creator;
        this->direction = Normalized(direction);
        fPos = pos + Vec2(0.01f, 0.01f);
        this->pos = pos;
        begin = tTime;
        Start();
    }

    Entity* Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return new Projectile(this, pos, direction, creator);
    }

    void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
    {
        if(tTime - begin >= duration / speed)
            return DestroySelf(entities, this);

        if (CheckPos((Entities*)entities))
            return;
        Vec2 oldPos = pos;
        MovePos(dTime);
        if (oldPos != pos)
            CheckPos((Entities*)entities);
    }

    bool CheckPos(Entities* entities)
    {
        vector<Entity*>::iterator hitEntityPos = entities->FindCorpPos(pos);

        if (hitEntityPos == entities->corporeals.end() || *hitEntityPos == creator)
            return false;

        fPos -= direction;
        pos = Vec2(roundf(fPos.x), roundf(fPos.y));

        Entity* entity = *hitEntityPos;
        entity->DealDamage(damage, (vector<Entity*>*)entities, this);
        DestroySelf(entities, entity);
        return true;
    }

    virtual void MovePos(float dTime)
    {
        fPos += direction * dTime * speed;
        pos = Vec2(roundf(fPos.x), roundf(fPos.y));
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

    ShotItem(Item item, float speed = 8.0f, int mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(item.range, item.damage, speed, item.color, mass, maxHealth, health), item(item)
    {
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec2 pos, Vec2f direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        damage = item.damage;
        begin = tTime;
        this->creator = creator;
        fPos = pos + Vec2(0.01f, 0.01f);
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
        fPos = pos + Vec2(0.01f, 0.01f);
        float magnitude = Magnitude(direction);
        this->direction = direction / magnitude;
        duration = fminf(item.range, magnitude);
        this->pos = pos;
        Start();
    }

    Entity* Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return new ShotItem(this, pos, direction, creator);
    }

    void OnDeath(vector<Entity*>* entities, Entity* damageDealer) override
    {
        item.baseClass->OnDeath((vector<void*>*)(&((Entities*)entities)->collectibles), (vector<void*>*)entities, pos);
    }
};

namespace Projectiles
{
    Item* basicBullet = new Item("Basic bullet", olc::VERY_DARK_GREY, 2, 1, 30.0f);
}

ShotItem* basicShotItem = new ShotItem(*Resources::copper, 12, 1, 1, 1);