#pragma once

struct Vector2
{
	unsigned int x;
	unsigned int y;
};

struct Vector2DB
{
	unsigned int x : 8;
	unsigned int y : 8;
};

struct Vector2B
{
	unsigned int x : 4;
	unsigned int y : 4;
};