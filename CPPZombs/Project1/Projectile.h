#include "Entity.h"

class Projectile : public Entity
{
public:
    Vec2f direction;
    Vec2f fPos;
    int remainingTime;
    int damage;
    float speed, lastMove;

    Projectile(int maxDist, int damage = 1, float speed = 0.0625f, Color color = olc::GREY, int mass = 1, int maxHealth = 1, int health = 1) :
        Entity(Vec2(0, 0), color, mass, maxHealth, health),
        remainingTime(maxDist), damage(damage), speed(speed), lastMove(tTime - speed)
    {
        Start();
    }

    Projectile(Projectile* baseClass, Vec2 pos, Vec2 direction, Entity* creator):
        Projectile(*baseClass)
    {
        this->creator = creator;
        this->direction = (Vec2f)(direction) / Squagnitude(direction);
        fPos = pos + Vec2(0.01f, 0.01f);
        this->pos = pos;
        Start();
    }

    Entity* Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return new Projectile(this, pos, direction, creator);
    }

    void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
    {
        if (tTime - lastMove >= speed)
        {
            lastMove = tTime;
            remainingTime--;
            if (remainingTime < 0)
                return DestroySelf(entities, this);

            CheckPos((Entities*)entities);

            fPos += direction;
            pos = Vec2(roundf(fPos.x), roundf(fPos.y));
        }

        CheckPos((Entities*)entities);
    }

    void CheckPos(Entities* entities)
    {
        vector<Entity*>::iterator hitEntityPos = entities->FindCorpPos(pos);

        if (hitEntityPos != entities->corporeals.end() && *hitEntityPos != creator)
        {
            fPos -= direction;
            pos = Vec2(roundf(fPos.x), roundf(fPos.y));

            Entity* entity = *hitEntityPos;
            entity->DealDamage(damage, (vector<Entity*>*)entities, this);
            DestroySelf(entities, entity);
            return;
        }
    }

    virtual Vec2 MoveDir()
    {
        fPos += direction;
        return Vec2(roundf(fPos.x), roundf(fPos.y)) - pos;
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

    ShotItem(Item item, int maxDist = 10, float speed = 0.0625f, int mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(maxDist, item.damage, speed, item.color, mass, maxHealth, health), item(item)
    {
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec2 pos, Vec2 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        damage = item.damage;
        int magnitude = Squagnitude(direction);
        remainingTime = remainingTime < magnitude ? remainingTime : magnitude;
        this->creator = creator;
        fPos = pos + Vec2(0.01f, 0.01f);
        this->direction = (Vec2f)(direction) / magnitude;
        this->pos = pos;
        Start();
    }

    ShotItem(ShotItem* baseClass, Item item, Vec2 pos, Vec2 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        this->item = item;
        damage = item.damage;
        color = item.color;
        int magnitude = Squagnitude(direction);
        remainingTime = remainingTime < magnitude ? remainingTime : magnitude;
        this->creator = creator;
        fPos = pos + Vec2(0.01f, 0.01f);
        this->direction = (Vec2f)(direction) / magnitude;
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
    Projectile* basicBullet = new Projectile(10, 1, 0.00625f, olc::GREY, 1, 1, 1);
}

ShotItem* basicShotItem = new ShotItem(*Resources::copper, 10, 0.0625f, 1, 1, 1);