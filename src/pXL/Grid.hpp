#pragma once

#include <vector>
#include <cstdint>
#include <cassert>

#include <SFML/System.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace px
{
	template <typename T>
	class Grid
	{
	public:

		Grid(sf::Vector2u size, const T& base) :
			m_flat((size_t)size.x * size.y, base),
			m_size(size),
			m_base(base)
		{}

		sf::Vector2u size() const { return m_size; }
		const T& base() const { return m_base; }

		T& at(const sf::Vector2u position)
		{
			assert(withinBounds(position));
			return m_flat[m_size.x * position.y + position.x];
		}

		const T& at(const sf::Vector2u position) const
		{
			assert(withinBounds(position));
			return m_flat[m_size.x * position.y + position.x];
		}

		bool withinBounds(sf::Vector2u position) const
		{
			return position.x < m_size.x && position.y < m_size.y;
		}

		bool validRectangle(sf::Rect<uint32_t> rect) const
		{
			return rect.position.x <= m_size.x &&
				rect.position.y <= m_size.y &&
				rect.size.x <= m_size.x - rect.position.x &&
				rect.size.y <= m_size.y - rect.position.y;
		}

		void resize(const sf::Vector2u size)
		{
			if (size == m_size) return;

			std::vector<T> temp(size.x * size.y, m_base);
			for (size_t y = 0; y < m_size.y && y < size.y; ++y)
			{
				for (size_t x = 0; x < m_size.x && x < size.x; ++x)
				{
					temp[size.x * y + x] = m_flat[m_size.x * y + x];
				}
			}

			m_flat = temp;
			m_size = size;
		}

		void fill(sf::Rect<uint32_t> rectangle, const T& filler)
		{
			sf::Vector2u end = rectangle.position + rectangle.size;

			assert(validRectangle(rectangle));

			for (size_t y = rectangle.position.y; y < end.y; ++y)
			{
				for (size_t x = rectangle.position.x; x < end.x; ++x)
				{
					m_flat[m_size.x * y + x] = filler;
				}
			}
		}

		void clear(sf::Rect<uint32_t> rectangle)
		{
			fill(rectangle, m_base);
		}

		void paste(sf::Vector2u position, const Grid& grid)
		{
			sf::Rect<uint32_t> pasteRectangle(position, position + grid.m_size);
			assert(validRectangle(pasteRectangle));

			for (size_t y = position.y; y < pasteRectangle.size.y; ++y)
			{
				for (size_t x = position.x; x < pasteRectangle.size.x; ++x)
				{
					m_flat[m_size.x * (y + position.y) + (x + position.x)] = grid.m_flat[m_size.x * y + x];
				}
			}
		}

		Grid copy(sf::Rect<uint32_t> rectangle) const
		{
			sf::Vector2u end = rectangle.position + rectangle.size;

			assert(validRectangle(rectangle));

			Grid<T> out(rectangle.size, m_base);

			for (size_t y = 0; y < rectangle.size.y; ++y)
			{
				for (size_t x = 0; x < rectangle.size.x; ++x)
				{
					out.m_flat[out.m_size.x * y + x] = m_flat[m_size.x * (y + rectangle.position.y) + (x + rectangle.position.x)];
				}
			}

			return out;
		}

		Grid cut(sf::Rect<uint32_t> rectangle)
		{
			Grid<T> out = copy(rectangle);
			fill(rectangle, m_base);
			return out;
		}

	private:

		std::vector<T> m_flat;
		sf::Vector2u m_size;
		T m_base;
	};
}