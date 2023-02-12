#include "Enemy.h"

class Player : public LightBlock
{
public:
	Entity* currentMenuedEntity = nullptr;
	Items items;
	int vacDist;
	bool placedBlock;
	Vec2f placingDir = up;
	float timePerMove = 0.125f, lastMove = 0.0f, maxSpeed = 4.0f, lastVac = -1.0f,
		timePerVac = 0.125f, lastClick = 0.0f,
		timePerHoldMove = timePerMove, lastHoldMove = 0.0f;

	Player(Vec2 pos = vZero, Vec2 dimensions = vOne, int vacDist = 6, RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127),
		bool lightOrDark = true, RGBA subScat = RGBA(), float range = 10, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		LightBlock(lightColor, lightOrDark, range, pos, dimensions, color, color2, subScat, mass, maxHealth, health, name), vacDist(vacDist)
	{
		Start();
	}

	void Start() override
	{
		LightBlock::Start();
		items = Items();
		items.push_back(Resources::copper->Clone(10));
		items.push_back(Resources::cheese->Clone(3));
		items.push_back(Resources::shades->Clone(3));
		items.push_back(Resources::Seeds::copperTreeSeed->Clone(2));
		for (Item* item : Resources::Seeds::plantSeeds)
			items.push_back(item->Clone());
		items.currentIndex = 0; // Copper
	}

	void Update() override
	{
		bool exitedMenu = false;
		if (currentMenuedEntity != nullptr && (game->inputs.leftMouse.pressed || game->inputs.rightMouse.pressed)/* &&
			!currentMenuedEntity->PosInUIBounds(game->inputs.mousePosition + pos)*/)
		{
			currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = nullptr;
			exitedMenu = true;
			lastClick = tTime;
		}

		vector<Entity*> hitEntities;
		if (game->inputs.rightMouse.pressed && game->inputs.mousePosition != vZero &&
			(currentMenuedEntity == nullptr || !currentMenuedEntity->Overlaps(game->inputs.mousePosition + pos, vOne))
			&& (hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition + pos, dimensions)).size())
		{
			if (currentMenuedEntity != nullptr)
				currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = hitEntities[0];
			currentMenuedEntity->shouldUI = true;
		}

		#pragma region Movement

		if (game->inputs.w.pressed || game->inputs.up.pressed || game->inputs.a.pressed || game->inputs.left.pressed ||
			game->inputs.s.pressed || game->inputs.down.pressed || game->inputs.d.pressed || game->inputs.right.pressed)
			lastMove = tTime;

		if (tTime - lastMove >= timePerMove)
		{
			Vec2 direction(0, 0);

			#pragma region Inputs
			if (game->inputs.a.held || game->inputs.left.held)
			{
				game->inputs.a.held = false;
				game->inputs.left.held = false;
				direction.x--;
			}
			if (game->inputs.d.held || game->inputs.right.held)
			{
				game->inputs.d.held = false;
				game->inputs.right.held = false;
				direction.x++;
			}
			if (game->inputs.s.held || game->inputs.down.held)
			{
				game->inputs.s.held = false;
				game->inputs.down.held = false;
				direction.y--;
			}
			if (game->inputs.w.held || game->inputs.up.held)
			{
				game->inputs.w.held = false;
				game->inputs.up.held = false;
				direction.y++;
			}
			#pragma endregion

			Vec2 oldPos = pos;
			if (direction != vZero)
			{
				lastMove = tTime;
				TryMove(direction, 3);
			}

			game->inputs.mousePosition += pos - oldPos;
		}

		#pragma endregion

		if (heldEntity == nullptr && items.size() > 0) // You can't mod by 0.
			items.currentIndex = JMod(items.currentIndex + game->inputs.mouseScroll, static_cast<int>(items.size()));

		Item currentShootingItem = items.GetCurrentItem();

		if (game->inputs.middleMouse.released && heldEntity != nullptr)
		{
			heldEntity->holder = nullptr;
			heldEntity = nullptr;
		}

		if (heldEntity != nullptr)
		{
			heldEntity->dir.RotateLeft(game->inputs.mouseScroll);
			if (tTime - lastHoldMove > timePerHoldMove)
			{
				lastHoldMove = tTime;
				heldEntity->TryMove(Vec2f(pos + game->inputs.mousePosition - heldEntity->pos).Rormalized(), mass);
			}
		}
		
		
		Vec2 normalizedDir = Vec2f(game->inputs.mousePosition).Normalized();
		if (heldEntity == nullptr && game->inputs.middleMouse.pressed && game->inputs.mousePosition != vZero &&
			(hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition + pos, vOne)).size())
		{
			heldEntity = hitEntities[0];
			heldEntity->holder = this;
		}
		else if (!game->inputs.space.held && tTime - lastClick > currentShootingItem.shootSpeed && currentMenuedEntity == nullptr &&
			game->inputs.mousePosition != vZero && currentShootingItem != *dItem && game->inputs.leftMouse.held && items.TryTake(currentShootingItem))
		{
			lastClick = tTime;
			game->entities->push_back(basicShotItem->Clone(currentShootingItem,
				pos, game->inputs.mousePosition, this));
		}

		if (heldEntity == nullptr && game->inputs.space.held && tTime - lastVac > timePerVac)
		{
			game->entities->Vacuum(pos, vacDist);
			lastVac = tTime;
		}

		vector<Entity*> collectibles = EntitiesOverlaps(pos, dimensions, game->entities->collectibles);
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