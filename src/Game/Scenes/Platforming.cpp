#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <optional>

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

	{
		auto player = m_ctx.entities.get("player").spawn(m_registry);
		auto& transform = m_registry.get<Transform>(player);
		transform.pos = { 2.5f, 2.5f };
		transform.oldPos = transform.pos;
	}

	{
		auto spike = m_ctx.entities.get("spike").spawn(m_registry);
		auto& transform = m_registry.get<Transform>(spike);
		transform.pos = {8.5f, 8.0f};
		transform.oldPos = transform.pos;
	}

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
	precomputeHitboxes(ctx);

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

		/*ctx.window.setView(ctx.window.getDefaultView());
		sf::Text positionLavel(api.assets.font, std::to_string(transform.pos.x) + ", " + std::to_string(transform.pos.y));
		ctx.window.draw(positionLavel);*/
	});

	auto hitboxView = m_registry.view<const Transform, const Hitbox>();

	hitboxView.each([&](const Transform& transform, const Hitbox& hitbox)
	{
		sf::RectangleShape hitboxRectangle{ hitbox.rect.size * unitPixels };
		sf::Vector2f position = px::lerp(
			hitbox.rect.position + transform.oldPos,
			hitbox.rect.position + transform.pos,
			ctx.alpha
		) * static_cast<float>(unitPixels);
		hitboxRectangle.setPosition(position);
		hitboxRectangle.setFillColor(sf::Color(255, 0, 0, 150));

		ctx.window.draw(hitboxRectangle);
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
			controllable.jumpBuffer = sf::seconds(999);
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
	struct Colider
	{
		float time;
		sf::FloatRect rectangle;
		std::optional<entt::entity> entity;

		bool operator<(const Colider& o)
		{
			return time < o.time;
		}
	};

	auto view = m_registry.view<Transform, const Hitbox>();

	view.each([&](entt::entity entity, Transform& transform, const Hitbox& hitbox)
	{
		transform.oldPos = transform.pos;

		sf::FloatRect entityRect = hitbox.rect;
		entityRect.position += transform.pos;
		sf::Vector2u size = m_map.size();
		sf::Vector2f deltaDistance = transform.vel * ctx.dt.asSeconds();

		static std::vector<Colider> coliders;
		coliders.clear();

		for (uint32_t y{}; y < size.y; ++y)
		{
			for (uint32_t x{}; x < size.x; ++x)
			{
				if (m_map.at({ x,y }).type == Tile::Type::Air)
				{
					continue;
				}

				sf::FloatRect tileRect{ {static_cast<float>(x), static_cast<float>(y)}, {1.0f, 1.0f} };
				px::ColisionResult result = px::sweptAABB(entityRect, tileRect, deltaDistance);

				if (!result.hit)
				{
					continue;
				}

				coliders.push_back({ result.time, tileRect });
			}
		}

		view.each([&](entt::entity innerEntity, Transform& _, const Hitbox& innerHitbox)
		{
			if (entity == innerEntity)
			{
				return;
			}

			px::ColisionResult result = px::sweptAABB(entityRect, innerHitbox.precomputed, deltaDistance);

			if (!result.hit)
			{
				return;
			}

			coliders.push_back({ result.time, innerHitbox.precomputed, innerEntity });
		});

		std::sort(coliders.begin(), coliders.end());

		for (const auto& colider : coliders)
		{
			px::ColisionResult result = px::sweptAABB(entityRect, colider.rectangle, deltaDistance);

			if (!result.hit)
			{
				continue;
			}

			if (colider.entity && m_registry.all_of<ColiderType>(colider.entity.value())
				&& m_registry.get<ColiderType>(colider.entity.value()) == ColiderType::Hazard
				&& !ctx.transition.isActive())
			{
				ctx.transition.start([&]()
				{
					api.comms.replace("Platforming");
				});
			}

			transform.vel += sf::Vector2f{
				result.normal.x * std::abs(transform.vel.x),
				result.normal.y * std::abs(transform.vel.y)
			} * (1.0f - result.time);

			deltaDistance = transform.vel * ctx.dt.asSeconds();
		}

		transform.pos += transform.vel * ctx.dt.asSeconds();

		if (m_registry.all_of<Controllable>(entity))
		{
			m_oldCameraPosition = m_cameraPosition;
			m_cameraPosition = px::lerp(m_cameraPosition, { transform.pos.x + m_dir * 1.0f, transform.pos.y - 1.0f }, 0.05f);
		}
	});

	auto particleView = m_registry.view<Transform, IsParticle>();

	particleView.each([&](auto& transform) {
		transform.oldPos = transform.pos;
		transform.pos += transform.vel * ctx.dt.asSeconds();
	});
}