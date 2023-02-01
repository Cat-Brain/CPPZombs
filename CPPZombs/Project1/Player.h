#include "Enemy.h"

class Player : public LightBlock
{
public:
	Entity* currentMenuedEntity = nullptr;
	Items items;
	int vacDist;
	bool placedBlock;
	Vec2f placingDir = up;
	float moveSpeed = 16, maxSpeed = 4.0f, lastVac = -1.0f, vacSpeed = 32.0f, lastClick = -1.0f, clickSpeed = 0.02f;

	Player(Vec2 pos = vZero, Vec2 dimensions = vOne, int vacDist = 6, RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127),
		RGBA subsurfaceResistance = RGBA(), int lightFalloff = 50, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		LightBlock(lightColor, lightFalloff, pos, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name), vacDist(vacDist)
	{
		Start();
	}

	void Start() override
	{
		LightBlock::Start();
		items = Items();
		//items.push_back(Resources::copper->Clone(10));
		items.push_back(Resources::emerald->Clone(600));
		items.push_back(Resources::lead->Clone(20));
		//items.push_back(Resources::cheese->Clone(10));
		items.push_back(Resources::Seeds::copperTreeSeed->Clone(2));
		for (Item* item : Resources::Seeds::plantSeeds)
			items.push_back(item->Clone());
		items.currentIndex = 0; // Copper
	}

	void ReduceVel() override
	{
		vel *= powf(0.1f, game->dTime);
	}

	void Update() override
	{
		bool exitedMenu = false;
		if (currentMenuedEntity != nullptr && (game->inputs.leftMouse.pressed || game->inputs.rightMouse.pressed) &&
			!currentMenuedEntity->PosInUIBounds(game->inputs.mousePosition + iPos))
		{
			currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = nullptr;
			exitedMenu = true;
			lastClick = tTime;
		}

		vector<Entity*> hitEntities;
		if (game->inputs.rightMouse.pressed && game->inputs.mousePosition != vZero &&
			(currentMenuedEntity == nullptr || !currentMenuedEntity->IOverlaps(game->inputs.mousePosition + iPos, vOne))
			&& (hitEntities = game->entities->FindCorpIOverlaps(game->inputs.mousePosition + iPos, dimensions)).size())
		{
			if (currentMenuedEntity != nullptr)
				currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = hitEntities[0];
			currentMenuedEntity->shouldUI = true;
		}

		#pragma region Movement

		Vec2 force = vZero;

		if (game->inputs.a.held || game->inputs.left.held)
			vel.x--;
		if (game->inputs.d.held || game->inputs.right.held)
			vel.x++;
		if (game->inputs.s.held || game->inputs.down.held)
			vel.y--;
		if (game->inputs.w.held || game->inputs.up.held)
			vel.y++;

		Vec2 newVel = vel + force * moveSpeed * game->dTime;
		if (vel.SqrMagnitude() < maxSpeed * maxSpeed || newVel.SqrMagnitude() < vel.SqrMagnitude())
			vel = newVel;

		#pragma endregion

		if (heldEntity == nullptr && items.size() > 0) // You can't mod by 0.
			items.currentIndex = JMod(items.currentIndex + game->inputs.mouseScroll, static_cast<int>(items.size()));

		Item currentShootingItem = items.GetCurrentItem();

		if (game->inputs.middleMouse.released && heldEntity != nullptr)
		{
			heldEntity->holder = nullptr;
			heldEntity->vel = 0;
			heldEntity = nullptr;
		}

		if (heldEntity != nullptr)
		{
			heldEntity->dir.RotateLeft(game->inputs.mouseScroll);
			if (Vec2(heldEntity->pos) == game->inputs.mousePosition + iPos)
				heldEntity->vel = 0;
			else
				heldEntity->vel += (iPos + game->inputs.mousePosition - heldEntity->iPos).Normalized() / heldEntity->mass * 10;
		}
		
		
		Vec2 normalizedDir = Vec2(game->inputs.mousePosition).Normalized();
		if (heldEntity == nullptr && game->inputs.middleMouse.pressed && game->inputs.mousePosition != vZero &&
			(hitEntities = game->entities->FindCorpIOverlaps(game->inputs.mousePosition + iPos, vOne)).size())
		{
			heldEntity = hitEntities[0];
			heldEntity->holder = this;
		}
		else if (!game->inputs.space.held && tTime - lastClick > clickSpeed && currentMenuedEntity == nullptr &&
			game->inputs.mousePosition != vZero && currentShootingItem != *dItem && game->inputs.leftMouse.held && items.TryTake(currentShootingItem))
		{
			lastClick = tTime;
			game->entities->push_back(basicShotItem->Clone(currentShootingItem,
				iPos + dimensions / 2.0f + right * int(game->inputs.mousePosition.x <= 0) + up * int(game->inputs.mousePosition.y <= 0), game->inputs.mousePosition, this));
		}

		if (heldEntity == nullptr && game->inputs.space.held)
		{
			game->entities->Vacuum(iPos, vacSpeed, vacDist);
		}

		vector<Entity*> collectibles = EntitiesOverlaps(iPos, dimensions, game->entities->collectibles);
		for (Entity* collectible : collectibles)
		{
			items.push_back(((Collectible*)collectible)->baseItem);
			collectible->DestroySelf(this);
		}

		game->inputs.mouseScroll = 0;
	}

	void OnDeath(Entity* damageDealer) override
	{
		LightBlock::OnDeath(damageDealer);
		playerAlive = false;
		if (damageDealer != nullptr)
			deathCauseName = damageDealer->name;
		else
			deathCauseName = "The planet";
	}
};