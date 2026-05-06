#pragma once

#include <cassert>
#include <algorithm>

#include <SFML/System.hpp>
#include <entt/entt.hpp>

class ColisionHelper
{
public:

	ColisionHelper(const entt::registry& registry) :
		m_registry(&registry)
	{
		calculateMinimumSlide();
	}

	void calculateMinimumSlide()
	{
		auto view = m_registry->view<Hitbox>();

		view.each([&](const auto& hitbox) {
			m_minimumSlide.x = std::min(m_minimumSlide.x, hitbox.rect.size.x);
			m_minimumSlide.y = std::min(m_minimumSlide.y, hitbox.rect.size.y);
		});

		m_minimumSlide.x -= 1e-6;
		m_minimumSlide.y -= 1e-6;
	}

	sf::Vector2f getMinimumSlide() const
	{
		return m_minimumSlide;
	}

private:

	const entt::registry* m_registry;
	sf::Vector2f m_minimumSlide;
};