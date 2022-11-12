#include "BuildingBlocks.h"

class Projectile : public Entity
{
public:
    Vec2f direction;
    Vec2f fPos;
    int remainingTime;

    Projectile(int maxDist, Color color, int mass, int maxHealth, int health) :
        Entity(Vec2(0, 0), color, mass, maxHealth, health), remainingTime(maxDist)
        { }

    Projectile(Projectile* baseClass, Entity* creator, Vec2 pos, Vec2 direction):
        Projectile(*baseClass)
    {
        this->creator = creator;
        fPos = pos + Vec2(0.01f, 0.01f);
        this->direction = (Vec2f)(direction - pos) / Squistance(pos, direction);
        this->pos = pos + playerVel;
    }

    void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
    {
        remainingTime--;
        if (remainingTime < 0)
            return DestroySelf(entities);

        Vec2 dir = MoveDir();
        Entity* entity;

        if (dir != Vec2(0, 0) && !TryMove(dir, 1, entities, &entity, creator))
        {
            entity->DealDamage(GetDamage(), entities);
            DestroySelf(entities);
            return;
        }

        Entity::Update(screen, entities, frameCount, inputs);
    }

    virtual Vec2 MoveDir()
    {
        fPos += direction;
        return Vec2(roundf(fPos.x), roundf(fPos.y)) - pos;
    }

    void OnDeath(vector<Entity*>* entities) override
    {
        ((Entities*)entities)->push_back(new MiniEntity(baseClass, pos));
    }

    virtual int GetDamage()
    {
        return 1;
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

Projectile* basicBullet = new Projectile(10, olc::GREY, 1, 1, 1);