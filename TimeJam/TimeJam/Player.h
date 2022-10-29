#pragma once

#include "Entity.h"

class Player : public Entity
{
public:
	void Update(float deltaTime, Input i) override;
};