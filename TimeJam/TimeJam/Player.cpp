#include "Player.h"

void Player::Update(float deltaTime, Input i)
{
	if (i.W)
		pos.x++;
	if (i.A)
		pos.y--;
	if (i.S)
		pos.x--;
	if (i.D)
		pos.y--;
}