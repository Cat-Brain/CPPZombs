#include "Entity.h"

class Projectile : public Entity
{
public:
    Vec2f direction;
    Vec2f fPos;
    int remainingTime;
    int damage;
    float speed, lastMove;

    Projectile(int maxDist, int damage = 1, float speed = 0.0625f, Color color = olc::GREY, Recipe cost = Recipes::dRecipe, int mass = 1, int maxHealth = 1, int health = 1) :
        Entity(Vec2(0, 0), color, cost, mass, maxHealth, health),
        remainingTime(maxDist), damage(damage), speed(speed), lastMove(tTime - speed)
    {
        Start();
    }

    Projectile(Projectile* baseClass, Vec2 pos, Vec2 direction, Entity* creator):
        Projectile(*baseClass)
    {
        this->creator = creator;
        fPos = pos + Vec2(0.01f, 0.01f);
        this->direction = (Vec2f)(direction) / Squagnitude(direction);
        this->pos = pos + playerVel;
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

            Vec2 dir = MoveDir();
            Entity* entity;

            if (dir != Vec2(0, 0) && !TryMove(dir, 1, entities, &entity, creator))
            {
                entity->DealDamage(damage, entities, this);
                DestroySelf(entities, entity);
                return;
            }
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
};


class ShotItem : public Projectile
{
public:
    Item item;

    ShotItem(Recipe cost, int maxDist = 10, float speed = 0.0625f, int mass = 1, int maxHealth = 1, int health = 1) :
        Projectile(maxDist, cost[0].damage, speed, cost[0].color, cost, mass, maxHealth, health)
    {
        Start();
    }

    ShotItem(ShotItem* baseClass, Vec2 pos, Vec2 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        damage = cost[0].damage;
        int magnitude = Squagnitude(direction);
        remainingTime = remainingTime < magnitude ? remainingTime : magnitude;
        this->creator = creator;
        fPos = pos + Vec2(0.01f, 0.01f);
        this->direction = (Vec2f)(direction) / magnitude;
        this->pos = pos + playerVel;
        Start();
    }

    ShotItem(ShotItem* baseClass, Recipe cost, Vec2 pos, Vec2 direction, Entity* creator) :
        ShotItem(*baseClass)
    {
        damage = cost[0].damage;
        this->cost = cost;
        this->color = cost[0].color;
        int magnitude = Squagnitude(direction);
        remainingTime = remainingTime < magnitude ? remainingTime : magnitude;
        this->creator = creator;
        fPos = pos + Vec2(0.01f, 0.01f);
        this->direction = (Vec2f)(direction) / magnitude;
        this->pos = pos + playerVel;
        Start();
    }

    Entity* Clone(Vec2 pos, Vec2 direction, Entity* creator) override
    {
        return new ShotItem(this, pos, direction, creator);
    }

    void OnDeath(vector<Entity*>* entities, Entity* damageDealer) override
    {
        for (int i = 0; i < cost.size(); i++)
            cost[i].baseClass->OnDeath((vector<void*>*)(&((Entities*)entities)->collectibles), (vector<void*>*)entities, pos);
    }
};

Projectile* basicBullet = new Projectile(10, 1, 0.00625f, olc::GREY, Recipes::basicBullet, 1, 1, 1);
ShotItem* basicShotItem = new ShotItem(Recipes::basicBullet, 10, 0.0625f, 1, 1, 1);