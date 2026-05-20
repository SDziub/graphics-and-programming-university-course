#pragma once

#include "pXL/pXL.hpp"

namespace Scenes
{
	class Pause : public px::Scene
	{
	public:

		Pause(px::SceneInitCtx& ctx) : Scene(ctx), m_menu({360, 260})
		{
			ctx.properties.setTransparency(true);

			m_menu.addButton("Resume", [&]() { api.comms.pop(); });
			m_menu.addButton("Exit", [&]() { api.comms.popUntil("MainMenu"); });
		}

		void update(px::UpdateCtx& ctx) override
		{
			if (api.mapping.isPressed("Pause"))
			{
				api.comms.pop({});
			}
			else if (api.mapping.isPressed("Up"))
			{
				m_menu.moveUp();
			}
			else if (api.mapping.isPressed("Down"))
			{
				m_menu.moveDown();
			}
			else if (api.mapping.isPressed("Confirm"))
			{
				m_menu.activate();
			}
		}

		void draw(px::DrawCtx& ctx) const override
		{
			sf::RectangleShape darkRect(static_cast<sf::Vector2f>(ctx.window.getSize()));
			darkRect.setFillColor(sf::Color(0, 0, 0, 100));

			ctx.window.draw(darkRect);

			sf::Text text(api.assets.font, "Pause", 72);
			
			ctx.window.draw(text);

			m_menu.draw(ctx);
		}

	private:

		px::TextMenu m_menu;
	};
}