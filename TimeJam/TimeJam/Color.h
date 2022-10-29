#pragma once

struct Color
{
	unsigned int r : 8;
	unsigned int g : 8;
	unsigned int b : 8;
};

struct ByteColor
{
	unsigned int r : 3;
	unsigned int g : 3;
	unsigned int b : 2;
};

Color BColToCol(ByteColor c)
{
	return { c.r * 32, c.g * 32, c.b * 64 };
}

ByteColor ColToBCol(Color c)
{
	return { c.r / 32, c.g / 32, c.b / 64 };
}