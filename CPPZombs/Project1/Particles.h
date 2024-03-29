#include "Entity.h"

class Particle
{
public:
	Vec3 pos;
	float startTime, duration;

	Particle(Vec3 pos, float duration) :
		pos(pos), duration(duration), startTime(tTime) { }

	~Particle() = default;

	virtual bool ShouldEnd()
	{
		return tTime - startTime > duration;
	}

	virtual void Update() { }
};

class Circle : public Particle
{
public:
	float radius;
	RGBA color;

	Circle(float radius, Vec3 pos, RGBA color, float duration) :
		Particle(pos, duration), radius(radius), color(color) { }

	void Update() override
	{
		if (color.a == 255)
			return game->DrawCircle(pos, color, radius);
		game->DrawCircle(pos, color, radius);
	}
};

class FadeCircle : public Circle
{
public:
	FadeCircle(float radius, Vec3 pos, RGBA color, float duration) :
		Circle(radius, pos, color, duration) { }

	void Update() override
	{
		//if (color.a == 255 && tTime == startTime)
		//	return game->DrawCircle(pos, color, radius);
		game->DrawCircle(pos, RGBA(color.r, color.g, color.b, static_cast<byte>(color.a * (1.f - (tTime - startTime) / duration))), radius);
	}
};

class TrackCircle : public Circle
{
public:
	Entity* entity;

	TrackCircle(Entity* entity, float radius, RGBA color, float duration) :
		Circle(radius, entity->pos, color, duration), entity(entity) { }

	void Update() override
	{
		pos = entity->pos;
		Circle::Update();
	}
};

class VelocityParticle : public Particle
{
public:
	Vec3 velocity;

	VelocityParticle(Vec3 pos, Vec3 velocity, float duration) :
		Particle(pos, duration), velocity(velocity) { }

	void Update() override
	{
		pos += velocity * game->dTime;
	}
};

class VelocityCircle : public VelocityParticle
{
public:
	float radius;
	RGBA color;

	VelocityCircle(float radius, Vec3 pos, Vec3 velocity, RGBA color, float duration) :
		VelocityParticle(pos, velocity, duration), radius(radius), color(color) { }

	void Update() override
	{
		VelocityParticle::Update();
		game->DrawCircle(pos, color, radius);
	}
};

class SpringCircle : public VelocityCircle
{
public:
	float strength, dampening;
	Vec3 desiredPos;

	SpringCircle(float strength, float dampening, float radius, Vec3 pos, Vec3 velocity, RGBA color, float duration) :
		VelocityCircle(radius, pos, velocity, color, duration), desiredPos(pos), strength(strength), dampening(dampening) { }

	void Update() override
	{
		velocity += (desiredPos - pos) * (strength * game->dTime) - velocity * (dampening * game->dTime);
		VelocityCircle::Update();
	}
};

class SpringFadeCircle : public SpringCircle
{
public:
	using SpringCircle::SpringCircle;

	void Update() override
	{
		byte tempAlpha = color.a;
		color.a = static_cast<byte>(color.a * (1.f - (tTime - startTime) / duration));
		SpringCircle::Update();
		color.a = tempAlpha;
	}
};

class SpinParticle : public Particle
{
public:
	float rotation, rotationalVelocity;

	SpinParticle(Vec3 pos, float duration, float rotation, float rotationalVelocity) :
		Particle(pos, duration), rotation(rotation), rotationalVelocity(rotationalVelocity) { }

	SpinParticle(Vec3 pos, float duration) :
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

	WobbleScaler(Vec3 pos, float duration, float scale, float wobbleSpeed, float wobbleStrength, float rotation, float rotationalVelocity) :
		SpinParticle(pos, duration, rotation, rotationalVelocity), scale(scale), baseScale(scale), wobbleSpeed(wobbleSpeed), wobbleStrength(wobbleStrength) { }

	WobbleScaler(Vec3 pos, float duration, float scale, float wobbleSpeed, float wobbleStrength) :
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

	SpinText(Vec3 pos, float duration, string text, RGBA color, float scale, float wobbleSpeed, float wobbleStrength, float rotation, float rotationalVelocity) :
		WobbleScaler(pos, duration, scale, wobbleSpeed, rotation, rotationalVelocity, wobbleStrength), text(text), color(color) { }

	SpinText(Vec3 pos, float duration, string text, RGBA color, float scale, float wobbleSpeed, float wobbleStrength) :
		WobbleScaler(pos, duration, scale, wobbleSpeed, wobbleStrength), text(text), color(color) { }

	void Update() override
	{
		WobbleScaler::Update();
		//font.RenderRotated(text, Vec2(pos - game->PlayerPos()) * Vec2(0.5f, 1) * Vec2(trueScreenWidth, trueScreenHeight) / game->zoom - scale / 2, rotation, scale, color);
	}
};

class LegParticle : public Particle
{
public:
	Entity* parent;
	Vec3 desiredPos;
	RGBA color;
	float moveSpeed;
	float thickness;

	LegParticle(Vec3 pos, Entity* parent, RGBA color, float moveSpeed, float thickness) :
		Particle(pos, 0), parent(parent), desiredPos(pos), color(color), moveSpeed(moveSpeed), thickness(thickness) { }

	bool ShouldEnd() override
	{
		return parent == nullptr;
	}

	void Update() override
	{
		pos = FromTo(pos, desiredPos, moveSpeed * game->dTime);
		if (parent != nullptr)
			game->DrawCapsule(pos, parent->pos, color, thickness);
	}
};