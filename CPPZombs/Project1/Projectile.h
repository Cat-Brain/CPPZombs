#include "BuildingBlocks.h"

class Projectile : public Entity
{
public:
    Vec2f direction;
    Vec2f fPos;
    int remainingTime;
    int whoDunIt;

    Projectile(Vec2 pos, Vec2 direction, int maxDist, int whoDunIt, Color color, int mass, int maxHealth, int health) :
        Entity(pos, color, mass, maxHealth, health), remainingTime(maxDist), whoDunIt(whoDunIt),
        fPos(pos + Vec2(0.01f, 0.01f))
    {
        this->direction = (Vec2f)(direction - pos) / Squistance(pos, direction);
        this->pos += playerVel;
    }

    void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
    {
        remainingTime--;
        if (remainingTime < 0)
            return DestroySelf(entities);

        Vec2 dir = MoveDir();
        int index;

        if (dir != Vec2(0, 0) && !TryMove(dir, 1, *entities, &index, whoDunIt))
        {
            (*entities)[index]->DealDamage(GetDamage(), entities);
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

    virtual void TryAdvance(Vec2 dir)
    {

    }

    virtual int GetDamage()
    {
        return 1;
    }

    bool IsProjectile() override
    {
        return true;
    }
};