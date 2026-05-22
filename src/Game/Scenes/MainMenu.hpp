#pragma once

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Audio.hpp>

#include "pXL/pXL.hpp"

namespace Scenes
{
	class MainMenu : public px::Scene
	{
	public:

		MainMenu(px::SceneInitCtx& ctx, sf::RenderWindow& window, Context& gctx) :
			Scene(ctx),
			m_gui(window),
			m_window(window),
			m_click(gctx.sounds.at("menu_click"))
		{
			m_gui.setFont("resources/Butterpop.otf");

			uint32_t y{};
			auto setButton = [&](tgui::Button::Ptr& button)
				{
					button->setPosition("(parent.width - width) / 2", ("parent.height / 2 - 120 + " + std::to_string(y)).c_str());
					y += 60;
					button->setSize("200", "50");

					auto renderer = button->getRenderer();
					renderer->setRoundedBorderRadius(40);
					renderer->setBorderColor({ 107, 62, 117 });
					renderer->setBackgroundColor({ 234, 173, 237 });
					renderer->setBackgroundColorDown({ 143, 211, 255 });
					renderer->setBackgroundColorHover({ 168, 132, 243 });

					m_gui.add(button);
				};

			auto button = tgui::Button::create("Play");
			button->onClick([&]()
				{
					api.transition.start([&]()
						{
							api.comms.push("Platforming");
						});
				});
			setButton(button);

			button = tgui::Button::create("Level Editor");
			button->onClick([&]()
				{
					api.transition.start([&]()
						{
							api.comms.push("LevelEditor");
						});
				});
			setButton(button);

			button = tgui::Button::create("Settings");
			button->onClick([&]()
				{
					api.transition.start([&]()
						{
							api.comms.push("Settings");
						});
				});
			setButton(button);

			button = tgui::Button::create("Exit");
			button->onClick([&]()
				{
					api.transition.start([&]()
						{
							m_window.close();
						});
				});
			setButton(button);
		}

		void onEvent(const sf::Event& event) override
		{
			m_gui.handleEvent(event);
		}

		void draw(px::DrawCtx& ctx) const override
		{
			ctx.window.draw(px::Background(api.assets.backgrounds.get("settings"), 0.f));

			ctx.window.setView(px::getRenderTargetView(ctx.window));
			sf::RectangleShape vignette(static_cast<sf::Vector2f>(ctx.window.getSize()));
			vignette.setTexture(&api.assets.textures.get("vignette"));
			vignette.setFillColor({ 0, 0, 0, 150 });
			ctx.window.draw(vignette);

			m_gui.draw();
		}

	private:

		mutable tgui::Gui m_gui;
		sf::Window& m_window;

		sf::Sound m_click;
	};
}