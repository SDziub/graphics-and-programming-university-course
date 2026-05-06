#pragma once

#include <cassert>

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
		assert(false && "This metod should not be called yet\n");
	}

	sf::Vector2f getMinimumSlide() const
	{
		return m_minimumSlide;
	}

private:

	const entt::registry* m_registry;
	sf::Vector2f m_minimumSlide;
};