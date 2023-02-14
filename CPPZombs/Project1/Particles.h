#include "Entity.h"

class Particle
{
public:
	Vec2f pos;
	float startTime, duration;

	Particle(Vec2f pos, float duration) :
		pos(pos), duration(duration), startTime(tTime) { }

	virtual bool ShouldEnd()
	{
		return tTime - startTime > duration;
	}

	virtual void Update() { }

	virtual void LowResUpdate() { }

	virtual void HighResUpdate() { }
};

class VelocityParticle : public Particle
{
public:
	Vec2f velocity;

	VelocityParticle(Vec2f pos, Vec2f velocity, float duration) :
		Particle(pos, duration), velocity(velocity) { }

	void Update() override
	{
		pos += velocity * game->dTime;
	}
};

class VelocitySquare : public VelocityParticle
{
public:
	RGBA color;

	VelocitySquare(Vec2f pos, Vec2f velocity, RGBA color, float duration) :
		VelocityParticle(pos, velocity, duration), color(color) { }

	void LowResUpdate() override
	{
		game->Draw(pos, color);
	}
};

class SpinParticle : public Particle
{
public:
	float rotation, rotationalVelocity;

	SpinParticle(Vec2f pos, float duration, float rotation, float rotationalVelocity) :
		Particle(pos, duration), rotation(rotation), rotationalVelocity(rotationalVelocity) { }

	SpinParticle(Vec2f pos, float duration) :
		Particle(pos, duration), rotation(RandFloat() * PI_F), rotationalVelocity((RandFloat() - 0.5f) * PI_F * 4.0f) { }

	void Update() override
	{
		rotation += game->dTime * rotationalVelocity;
	}
};

class WobbleScaler : public SpinParticle
{
public:
	float scale, baseScale, wobbleSpeed, wobbleStrength;

	WobbleScaler(Vec2f pos, float duration, float scale, float wobbleSpeed, float wobbleStrength, float rotation, float rotationalVelocity) :
		SpinParticle(pos, duration, rotation, rotationalVelocity), scale(scale), baseScale(scale), wobbleSpeed(wobbleSpeed), wobbleStrength(wobbleStrength) { }

	WobbleScaler(Vec2f pos, float duration, float scale, float wobbleSpeed, float wobbleStrength) :
		SpinParticle(pos, duration), scale(scale), baseScale(scale), wobbleSpeed(wobbleSpeed), wobbleStrength(wobbleStrength) { }

	void Update() override
	{
		SpinParticle::Update();
		scale = baseScale + sinf((tTime - startTime) * PI_F * wobbleSpeed) * wobbleStrength;
	}
};

class SpinText : public WobbleScaler
{
public:
	RGBA color;
	string text;

	SpinText(Vec2f pos, float duration, string text, RGBA color, float scale, float wobbleSpeed, float wobbleStrength, float rotation, float rotationalVelocity) :
		WobbleScaler(pos, duration, scale, wobbleSpeed, rotation, rotationalVelocity, wobbleStrength), text(text), color(color) { }

	SpinText(Vec2f pos, float duration, string text, RGBA color, float scale, float wobbleSpeed, float wobbleStrength) :
		WobbleScaler(pos, duration, scale, wobbleSpeed, wobbleStrength), text(text), color(color) { }

	void HighResUpdate() override
	{
		font.RenderRotated(text, (pos - game->PlayerPos()) * 2 * ScrDim() / midRes.ScrDim() - scale / 2, rotation, scale, color);
	}
};

class LegParticle : public Particle
{
public:
	Entity* parent;
	Vec2f desiredPos;
	RGBA color;
	float moveSpeed;

	LegParticle(Vec2f pos, Entity* parent, RGBA color, float moveSpeed) :
		Particle(pos, 0), parent(parent), desiredPos(pos), color(color), moveSpeed(moveSpeed) { }

	bool ShouldEnd() override
	{
		return parent == nullptr;
	}

	void Update() override
	{
		pos += (desiredPos - pos).Normalized().V2fMin(desiredPos - pos) * moveSpeed * game->dTime;
	}

	void LowResUpdate() override
	{
		game->DrawLine(pos, parent->pos, color);
	}
};