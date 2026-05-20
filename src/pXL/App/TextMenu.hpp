#pragma once

#include <string>
#include <vector>
#include <functional>
#include <utility>

#include <SFML/Graphics.hpp>

#include "pXL/Resources/Assets.hpp"

namespace px
{
	class TextMenu
	{
	public:

		using ButtonAction = std::function<void()>;
		
		TextMenu(sf::Vector2f position) : m_position(position) {}

		void addButton(const std::string& name, ButtonAction&& action)
		{
			m_buttons.emplace_back(name, std::move(action));
		}

		void activate() { m_buttons[m_current].second(); }
		void moveUp() { m_current = (m_current - 1 + m_buttons.size()) % m_buttons.size(); }
		void moveDown() { m_current = (m_current + 1) % m_buttons.size(); }

		void draw(const DrawCtx& api) const
		{
			size_t labels = m_buttons.size();
			sf::Vector2f position = m_position;
			for (size_t i = 0; i < labels; ++i)
			{
				sf::Text text(api.assets.font, m_buttons[i].first, 72);
				if (i == m_current)
				{
					text.setFillColor(sf::Color::Red);
				}
				sf::FloatRect bounds = text.getLocalBounds();
				position.x = m_position.x - bounds.size.x / 2;
				text.setPosition(position);

				api.window.draw(text);
				position.y += 72;
			}
		}

	private:

		std::vector<std::pair<std::string, ButtonAction>> m_buttons;
		size_t m_current{};
		sf::Vector2f m_position{ 0, 0 };
	};
}