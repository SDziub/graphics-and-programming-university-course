#pragma once

#include "pXL/pXL.hpp"

namespace Scenes
{
	class MainMenu : public px::Scene
	{
	public:

		MainMenu(px::SceneInitCtx& ctx, sf::Window& window) :
			Scene(ctx),
			m_menu({360, 360}),
			m_window(window)
		{
			m_menu.addButton("Play", [&]() { ctx.transition.start([&]() { api.comms.push("Platforming"); }); });
			m_menu.addButton("Level Editor", [&]() { ctx.transition.start([&]() { api.comms.push("LevelEditor"); }); });
			m_menu.addButton("Exit", [&]() { window.close(); });
		}

		void update(px::UpdateCtx& ctx) override
		{
			if (api.mapping.isPressed("Up"))
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
			ctx.window.clear(sf::Color(0x222222ff));

			const sf::Texture* mikuTexture = &api.assets.textures.get("player");
			sf::Vector2f mikuTextureSize = sf::Vector2f(mikuTexture->getSize());
			sf::RectangleShape mikuShape(mikuTextureSize);
			mikuShape.setTexture(mikuTexture);
			mikuShape.setOrigin(mikuTextureSize / 2.0f);
			mikuShape.setPosition(sf::Vector2f(ctx.window.getSize()) / 2.0f);
			mikuShape.setScale(api.scaling.getScale() * 4.0f);

			ctx.window.draw(mikuShape);

			m_menu.draw(ctx);
		}

	private:

		px::TextMenu m_menu;
		sf::Window& m_window;
	};
}