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
        this->direction = (direction - pos) / fmaxf(fabsf(direction.x - pos.x), fabsf(direction.y - pos.y));
    }

    void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
    {
        fPos += direction;
        Vec2 oldPos = pos;
        if(!TryMove(Vec2(roundf(fPos.x), roundf(fPos.y)) - pos, 1, *entities))
            printf("=[");

        Entity::Update(screen, entities, frameCount, inputs);
    }
};