#pragma once

#include "Color.h"
#include "Input.h"
#include "Vectors.h"

class Entity
{
public:
	Vector2DB pos;
	ByteColor color;


public:
	virtual void Update(float deltaTime, Input i);
};