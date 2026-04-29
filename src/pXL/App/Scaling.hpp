#pragma once

#include <SFML/Graphics.hpp>

namespace px
{
	class Engine;

	class Scaling
	{
	public:

		Scaling(const sf::Window& window) : 
			m_window(window)
		{
			update();
		}

		void setMinimumUnits(sf::Vector2f units)
		{
			m_minimumUnits = units;
		}

		sf::Vector2f getMinimumUnits() const
		{
			return m_minimumUnits;
		}

		void setUnitPixels(float size)
		{
			m_unitPixels = size;
		}

		float getUnitPixels() const
		{
			return m_unitPixels;
		}

		float getUnit() const
		{
			return m_unit;
		}

		sf::Vector2f getScale() const
		{
			return m_scale;
		}

		sf::Vector2u getMinimumWindowSize() const
		{
			return static_cast<sf::Vector2u>(m_minimumUnits * m_unitPixels);
		}

	private:

		void update()
		{
			sf::Vector2u minimumWindowSize = getMinimumWindowSize();
			sf::Vector2u windowSize = m_window.getSize();

			float ratio = std::min(
				floorf(windowSize.x / minimumWindowSize.x),
				floorf(windowSize.y / minimumWindowSize.y)
			);

			m_scale.x = ratio;
			m_scale.y = ratio;

			m_unit = m_unitPixels * ratio;
		}

		const sf::Window& m_window;
		sf::Vector2f m_minimumUnits{ 20.0f, 10.25f };
		float m_unitPixels{ 16 };
		sf::Vector2f m_scale;
		float m_unit;

		friend Engine;
	};
}