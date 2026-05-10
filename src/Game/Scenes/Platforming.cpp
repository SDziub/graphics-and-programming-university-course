#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>

#include "Platforming.hpp"
#include "Game/Constants.hpp"
#include "Game/ColisionHelper.hpp"

Scenes::Platforming::Platforming(px::SceneInitCtx& ctx, Context& gctx) :
	Scene(ctx),
	m_ctx(gctx),
	m_map(sf::Vector2u(40, 20), m_ctx.tiles["empty"]),
	m_colisionHelper(m_registry)
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

	m_colisionHelper.calculateMaxSlide();
}

void Scenes::Platforming::update(px::UpdateCtx& ctx)
{
	m_elapsed += ctx.dt;

	if (api.mapping.isPressed("Pause"))
	{
		api.comms.push("Pause");
	}

	animate(ctx);
}

void Scenes::Platforming::fixedUpdate(px::UpdateCtx& ctx)
{
	computeLifetime(ctx);

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
			sprite.setPosition(static_cast<sf::Vector2f>(position) * unitPixels);
			sprite.setScale(api.scaling.getScale());
			ctx.window.draw(sprite);
		}
	}

	auto view = m_registry.view<const px::Animation, const Transform>();

	view.each([&](const auto& animation, const auto& transform) {
		sf::Vector2f position = px::lerp(transform.oldPos, transform.pos, ctx.alpha) * static_cast<float>(unitPixels);

		sf::Sprite sprite = animation.getSprite().value();
		sprite.setPosition(position);
		sprite.setScale(api.scaling.getScale());
		ctx.window.draw(sprite);
	});
}

void Scenes::Platforming::animate(px::UpdateCtx& ctx)
{
	auto view = m_registry.view<px::Animation>();

	view.each([&](auto& animation) {
		animation.setMirrored(m_dir == -1);
		animation.update(ctx.dt);
	});

	auto playerView = m_registry.view<const Controllable, const Transform, px::Animation>();

	playerView.each([&](const auto& controllable, const auto& transform, auto& animation) {
		if (!controllable.grounded)
		{
			if (transform.vel.y < 0.0f)
			{
				animation.play("jump");
				return;
			}
			animation.play("fall");
			return;
		}
		if (transform.vel.x == 0.0f)
		{
			animation.play("idle");
			return;
		}
		animation.play("run");
	});
}

void Scenes::Platforming::computeLifetime(px::UpdateCtx ctx)
{
	auto view = m_registry.view<Lifetime>();

	view.each([&](entt::entity entity, auto& lifetime) {
		lifetime.lived += ctx.dt;
		if (lifetime.lived > lifetime.max)
		{
			m_registry.destroy(entity);
		}
	});
}

void Scenes::Platforming::playerControlSystem(px::UpdateCtx& ctx)
{
	auto view = m_registry.view<Controllable, Transform, const Hitbox>();

	view.each([&](auto& controllable, auto& transform, const auto& hitbox) {
		if (api.mapping.isPressed("Jump"))
		{
			controllable.jumpBuffer = sf::Time::Zero;
			transform.jumpStartY = transform.pos.y;
		}
		else
		{
			controllable.jumpBuffer += ctx.dt;
		}

		controllable.cayoteTime += ctx.dt;

		sf::Vector2 bottomLeft = transform.pos + hitbox.rect.position;
		bottomLeft.y += hitbox.rect.size.y;
		sf::Vector2f bottomRight = bottomLeft;
		bottomRight.x += hitbox.rect.size.x;

		bool wasGrounded = controllable.grounded;
		controllable.grounded = raycast(m_map, bottomLeft, { 0, 1.0f }, 1e-6).type == Tile::Type::Solid ||
			raycast(m_map, bottomRight, { 0, 1.0f }, 1e-6).type == Tile::Type::Solid;

		if (!wasGrounded && controllable.grounded && transform.pos.y > transform.jumpStartY - 1e-3)
		{
			for (size_t i = 0; i < 10; ++i)
			{
				auto particle = m_ctx.entities.get("cloud_particle").spawn(m_registry);
				auto& particleTransform = m_registry.get<Transform>(particle);

				particleTransform.pos = transform.pos;
				particleTransform.oldPos = transform.pos;

				float angle = static_cast<float>(rand()) / RAND_MAX * 2.f * M_PI;
				sf::Vector2f direction(cosf(angle), sinf(angle));

				particleTransform.vel = direction * 1.5f;
			}
		}
		
		if (controllable.grounded)
		{
			controllable.cayoteTime = sf::Time::Zero;
			controllable.canJump = true;
		}

		if (controllable.jumpBuffer <= k_bufferedJumpLimit && controllable.canJump && controllable.cayoteTime <= k_cayoteTime)
		{
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

void Scenes::Platforming::precomputeHitboxes(px::UpdateCtx& ctx)
{
	auto view = m_registry.view<const Transform, Hitbox>();

	view.each([](const auto& transform, auto& hitbox) {
		auto precomputed = hitbox.rect;
		precomputed.position += transform.pos;
		hitbox.precomputed = precomputed;
	});
}

void Scenes::Platforming::movementAndColisionSystem(px::UpdateCtx& ctx)
{
	auto view = m_registry.view<Transform, const Hitbox>();

	view.each([&](entt::entity entity, Transform& transform, const Hitbox& hitbox) {
		sf::Vector2f normalizedVelocity = transform.vel.normalized();
		sf::Vector2f maxSlide = m_colisionHelper.getMaxSlide();
		sf::Vector2f normalizedMaxSlide = maxSlide.normalized();

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
				}
			}

			transform.pos.y = currentY - (rect.size.y + rect.position.y);
		}

		m_oldCameraPosition = m_cameraPosition;
		m_cameraPosition = px::lerp(m_cameraPosition, { transform.pos.x + m_dir * 1.0f, transform.pos.y - 1.0f }, 0.05f);
	});

	auto particleView = m_registry.view<Transform, IsParticle>();

	particleView.each([&](auto& transform) {
		transform.oldPos = transform.pos;
		transform.pos += transform.vel * ctx.dt.asSeconds();
	});
}