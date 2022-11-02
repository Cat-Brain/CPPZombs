#include "BuildingBlocks.h"

class Projectile : public Entity
{
public:
    Vec2f direction;
    Vec2f fPos;
    int maxDist;

    Projectile(Vec2 pos, Vec2 direction, int maxDist, Color color, int mass, int maxHealth, int health) :
        Entity(pos, color, mass, maxHealth, health), maxDist(maxDist),
        fPos(pos + Vec2(0.01f, 0.01f))
    {
        this->direction = (Vec2f)(direction - pos) / Squistance(pos, direction);
    }

    void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
    {
        fPos += direction;
        Vec2 oldPos = pos;
        int index;
        if ((index = TryMove(Vec2(roundf(fPos.x), roundf(fPos.y)) - pos, 1, *entities)) != -1)
        {
            (*entities)[index]->health -= GetDamage();
            remove(entities->begin(), entities->end(), this);
            delete this;
            return;
        }

        Entity::Update(screen, entities, frameCount, inputs);
    }

    virtual int GetDamage()
    {
        return 1;
    }
};