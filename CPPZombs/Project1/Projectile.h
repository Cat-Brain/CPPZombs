#include "BuildingBlocks.h"

class Projectile : public Entity
{
public:
    float step;
    Vec2f direction;
    Vec2f fPos;
    int maxDist;

    Projectile(Vec2 pos, Vec2 direction, int maxDist, Color color, int mass, int maxHealth, int health) :
        Entity(pos, color, mass, maxHealth, health), maxDist(maxDist),
        step(fmaxf(fabsf(direction.x - pos.x), fabsf(direction.y - pos.y))), direction(Entity::ToSpace(direction - pos)), fPos(pos + Vec2(0.01f, 0.01f))
    {
        //printf("(%f, %f), %f, ", this->direction.x, direction.y, step);
        this->direction /= step;
        //printf("(%f, %f)\n", direction.x, direction.y);
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