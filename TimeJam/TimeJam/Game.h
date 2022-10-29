#pragma once

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "Player.h"

class Game : public olc::PixelGameEngine
{
public:
	Player player;
	Input input;

public:
	Game()
	{
		// Name your application
		sAppName = "Example";
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Called once per frame, draws random coloured pixels
		for (int x = 0; x < ScreenWidth(); x++)
			for (int y = 0; y < ScreenHeight(); y++)
				Draw(x, y, olc::Pixel(rand() % 256, rand() % 256, rand() % 256));

		input.W = GetKey(olc::W).bHeld;
		input.A = GetKey(olc::A).bHeld;
		input.S = GetKey(olc::S).bHeld;
		input.D = GetKey(olc::D).bHeld;
		input.Space = GetKey(olc::SPACE).bHeld;

		player.Update(fElapsedTime, input);
		Draw(player.pos.x, player.pos.y);

		return true;
	}
};