#include <algorithm>

#include "Platforming.hpp"
#include "Game/Constants.hpp"

Scenes::Platforming::Platforming(px::SceneInitCtx& ctx, Context& gctx) :
	Scene(ctx),
	m_ctx(gctx),
	m_map(sf::Vector2u(40, 20), m_ctx.tiles["empty"])
{
	const char* mapBuilder[]{
		"########################################",
		"#                                      #",
		"#                                ##    #",
		"#   ######          #####              #",
		"#                                      #",
		"#          ###               ##        #",
		"#                                      #",
		"#                       ##             #",
		"#### ##############                    #",
		"#                                      #",
		"#  #                  ####             #",
		"#     ##    #                          #",
		"#                           #          #",
		"#                                      #",
		"#    ##   ###                  #       #",
		"#                    #                 #",
		"##               #         #           #",
		"#                                      #",
		"#                                      #",
		"########################################",
	};

	for (uint32_t y = 0; y < m_map.size().y; ++y)
	{
		for (uint32_t x = 0; x < m_map.size().x; ++x)
		{
			if (mapBuilder[y][x] == '#')
			{
				m_map.at({ x, y }) = m_ctx.tiles.at("solid_block");
			}
		}
	}

	m_ctx.entities.get("player").spawn(m_registry);
}

void Scenes::Platforming::update(px::UpdateCtx& ctx)
{
	m_elapsed += ctx.dt;

	if (api.mapping.isPressed("Pause"))
	{
		api.comms.push("Pause");
	}

	advanceAnimation(ctx);
}

void Scenes::Platforming::fixedUpdate(px::UpdateCtx& ctx)
{
	playerControlSystem(ctx);

	movementAndColisionSystem(ctx);
}

void Scenes::Platforming::draw(px::DrawCtx& ctx) const
{
	ctx.window.clear(sf::Color::Blue);

	const sf::Vector2u size = m_map.size();
	const float unitPixels = api.scaling.getUnit();
	
	{
		sf::Vector2f windowSize = static_cast<sf::Vector2f>(ctx.window.getSize());
		sf::Vector2f halfScreenTiles = windowSize / static_cast<float>(unitPixels) / 2.0f;

		sf::Vector2f position = px::lerp(m_oldCameraPosition, m_cameraPosition, ctx.alpha);
		position.x = std::min(position.x, m_map.size().x - halfScreenTiles.x);
		position.y = std::min(position.y, m_map.size().y - halfScreenTiles.y);
		position.x = std::max(position.x, halfScreenTiles.x);
		position.y = std::max(position.y, halfScreenTiles.y);

		ctx.window.draw(px::Background(api.assets.backgrounds.get("background"), position.x * unitPixels));

		sf::View view(
			position * static_cast<float>(unitPixels),
			static_cast<sf::Vector2f>(ctx.window.getSize())
		);

		ctx.window.setView(view);
	}

	for (size_t y = 0; y < size.y; ++y) for (size_t x = 0; x < size.x; ++x)
	{
		sf::Vector2u position(x, y);

		if (m_map.at(position).sprite != "")
		{
			auto sprite(api.assets.tileSprites.get(m_map.at(position).sprite).get(getAdjacent(m_map, position), api.assets.textures));

			sprite.setPosition(sf::Vector2f{ x * unitPixels, y * unitPixels });

			sprite.setScale(api.scaling.getScale());

			ctx.window.draw(sprite);
		}
	}

	auto view = m_registry.view<const px::AnimatedSprite, const Transform>();

	view.each([&](const auto& sprite, const auto& transform) {
		sf::Vector2f position = px::lerp(transform.oldPos, transform.pos, ctx.alpha) * static_cast<float>(unitPixels);

		px::SpriteRenderer renderer(sprite);
		renderer.setPosition(position);
		renderer.setScale(api.scaling.getScale());
		ctx.window.draw(renderer);
	});
}

void Scenes::Platforming::advanceAnimation(px::UpdateCtx& ctx)
{
	auto view = m_registry.view<const Controllable, const Transform, px::AnimatedSprite>();
	view.each([&](const auto& controllable, const auto& transform, auto& sprite) {
		sprite.setMirrored(m_dir == -1);
		sprite.update(ctx.dt);

		if (!controllable.grounded)
		{
			if (transform.vel.y < 0.0f)
			{
				sprite.play("jump");
				return;
			}
			sprite.play("fall");
			return;
		}
		if (transform.vel.x == 0.0f)
		{
			sprite.play("idle");
			return;
		}
		sprite.play("run");
	});
}

void Scenes::Platforming::playerControlSystem(px::UpdateCtx& ctx)
{
	if (api.mapping.isPressed("Jump"))
	{
		m_jumpBuffer = sf::Time::Zero;
	}
	else if (m_jumpBuffer)
	{
		m_jumpBuffer.value() += ctx.dt;
	}

	m_floor += ctx.dt;

	auto view = m_registry.view<Controllable, Transform>();

	view.each([&](auto& controllable, auto& transform) {
		if (m_jumpBuffer && m_jumpBuffer.value() <= k_bufferedJumpLimit && controllable.canJump && m_floor <= k_cayoteTime)
		{
			m_jumpBuffer = {};
			transform.vel.y = -k_jumpVelocity;
			controllable.canJump = false;
		}
		else if (transform.vel.y < 0.0f)
		{
			transform.vel.y = std::min(transform.vel.y + k_downAcceleration * ctx.dt.asSeconds(), k_maxDownAcceleration);
		}
		else
		{
			transform.vel.y = std::min(transform.vel.y + k_downAcceleration * k_fallMultiplayer * ctx.dt.asSeconds(), k_maxDownAcceleration);
		}

		int32_t direction = 0 - api.mapping.isHeld("Left") + api.mapping.isHeld("Right");

		m_dir = direction != 0 ? direction : m_dir;

		transform.vel.x += (direction * k_acceleration * ctx.dt.asSeconds());

		if (!direction)
		{
			float newVelocity = std::abs(transform.vel.x) - k_deceleration * ctx.dt.asSeconds();

			if (newVelocity < 0.0f)
			{
				transform.vel.x = 0.0f;
				return;
			}

			transform.vel.x = (transform.vel.x > 0.0f ? 1.0f : -1.0f) * newVelocity;
			return;
		}

		if (std::abs(transform.vel.x) > k_maxSpeed)
		{
			transform.vel.x = (transform.vel.x > 0.0f ? 1.0f : -1.0f) * k_maxSpeed;
		}
	});
}

void Scenes::Platforming::movementAndColisionSystem(px::UpdateCtx& ctx)
{
	// The grounded check is stupid but what can you do? will fix it later

	auto view = m_registry.view<Transform, Hitbox, Controllable>();

	view.each([&](auto& transform, auto& hitbox, auto& controllable) {
		transform.oldPos = transform.pos;

		sf::FloatRect rect = hitbox.rect;

		int32_t minY = rect.position.y + transform.pos.y;
		int32_t maxY = rect.position.y + rect.size.y + transform.pos.y;

		if (transform.vel.x < 0.0f)
		{
			float currentX = rect.position.x + transform.pos.x;
			float possibleX = currentX + transform.vel.x * ctx.dt.asSeconds();

			bool colided = false;
			while (currentX - 1e-3f > possibleX && !colided)
			{
				for (size_t y = minY; y <= maxY; ++y)
				{
					if (m_map.at(sf::Vector2u(currentX - 1e-3f, y)).type != Tile::Type::Air)
					{
						colided = true;
						break;
					}
				}

				if (!colided)
				{
					currentX -= 1e-3f;
				}
			}

			transform.pos.x = currentX - rect.position.x;
		}
		else if (transform.vel.x > 0.0f)
		{
			float currentX = rect.position.x + rect.size.x + transform.pos.x;
			float possibleX = currentX + transform.vel.x * ctx.dt.asSeconds();

			bool colided = false;
			while (currentX + 1e-3f < possibleX && !colided)
			{
				for (size_t y = minY; y <= maxY; ++y)
				{
					if (m_map.at(sf::Vector2u(currentX + 1e-3f, y)).type != Tile::Type::Air)
					{
						colided = true;
						break;
					}
				}

				if (!colided)
				{
					currentX += 1e-3f;
				}
			}

			transform.pos.x = currentX - (rect.size.x + rect.position.x);
		}

		int32_t minX = rect.position.x + transform.pos.x;
		int32_t maxX = rect.position.x + rect.size.x + transform.pos.x;

		if (transform.vel.y < 0.0f)
		{
			float currentY = rect.position.y + transform.pos.y;
			float possibleY = currentY + transform.vel.y * ctx.dt.asSeconds();

			bool colided = false;
			while (currentY - 1e-3f > possibleY && !colided)
			{
				for (size_t x = minX; x <= maxX; ++x)
				{
					if (m_map.at(sf::Vector2u(x, currentY - 1e-3f)).type != Tile::Type::Air)
					{
						colided = true;
						break;
					}
				}

				if (!colided)
				{
					currentY -= 1e-3f;
				}
			}
			controllable.grounded = false;

			transform.pos.y = currentY - rect.position.y;
		}
		else if (transform.vel.y > 0.0f)
		{
			float currentY = rect.position.y + rect.size.y + transform.pos.y;
			float possibleY = currentY + transform.vel.y * ctx.dt.asSeconds();

			bool colided = false;
			while (currentY + 1e-3f < possibleY && !colided)
			{
				for (size_t x = minX; x <= maxX; ++x)
				{
					if (m_map.at(sf::Vector2u(x, currentY + 1e-3f)).type != Tile::Type::Air)
					{
						colided = true;
						break;
					}
				}

				if (!colided)
				{
					currentY += 1e-3f;
				}
				else
				{
					transform.vel.y = 0.0f;
					m_floor = sf::Time::Zero;
					controllable.canJump = true;
				}
			}
			controllable.grounded = colided;

			transform.pos.y = currentY - (rect.size.y + rect.position.y);
		}

		m_oldCameraPosition = m_cameraPosition;
		m_cameraPosition = px::lerp(m_cameraPosition, { transform.pos.x + m_dir * 1.0f, transform.pos.y - 1.0f }, 0.05f);
	});
}