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

		float getScalar() const
		{
			return m_scale.x;
		}

		sf::Vector2f getScale() const
		{
			return m_scale;
		}

		float getTrueUnit() const
		{
			return m_trueUnit;
		}

		float getTrueScalar() const
		{
			return m_trueScale.x;
		}

		sf::Vector2f getTrueScale() const
		{
			return m_trueScale;
		}

		sf::Vector2u getMinimumWindowSize() const
		{
			return static_cast<sf::Vector2u>(m_minimumUnits * m_unitPixels);
		}

	private:

		void update()
		{
			auto minimumWindowSize = static_cast<sf::Vector2f>(getMinimumWindowSize());
			auto windowSize = static_cast<sf::Vector2f>(m_window.getSize());

			float ratio = std::min(
				windowSize.x / minimumWindowSize.x,
				windowSize.y / minimumWindowSize.y
			);

			m_trueScale.x = ratio;
			m_trueScale.y = ratio;

			m_trueUnit = m_unitPixels * ratio;

			float ratioFloored = floorf(ratio);

			m_scale.x = ratioFloored;
			m_scale.y = ratioFloored;

			m_unit = m_unitPixels * ratioFloored;
		}

		const sf::Window& m_window;
		sf::Vector2f m_minimumUnits{ 20.0f, 10.25f };
		float m_unitPixels{ 16 };
		sf::Vector2f m_scale;
		float m_unit;
		sf::Vector2f m_trueScale;
		float m_trueUnit;

		friend Engine;
	};
}