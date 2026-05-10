#pragma once

#include <cassert>
#include <algorithm>

#include <SFML/System.hpp>
#include <entt/entt.hpp>

class ColisionHelper
{
public:

	ColisionHelper(const entt::registry& registry) :
		m_registry(&registry),
		m_maxSlide(1.0f, 1.0f)
	{}

	void calculateMaxSlide()
	{
		auto view = m_registry->view<Hitbox>();

		view.each([&](const auto& hitbox) {
			m_maxSlide.x = std::min(m_maxSlide.x, hitbox.rect.size.x);
			m_maxSlide.y = std::min(m_maxSlide.y, hitbox.rect.size.y);
		});

		m_maxSlide.x -= 1e-6;
		m_maxSlide.y -= 1e-6;
	}

	sf::Vector2f getMaxSlide() const
	{
		return m_maxSlide;
	}

private:

	const entt::registry* m_registry;
	sf::Vector2f m_maxSlide;
};