#pragma once

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "pXL/pXL.hpp"

namespace Scenes
{
	class MainMenu : public px::Scene
	{
	public:

		MainMenu(px::SceneInitCtx& ctx, sf::RenderWindow& window) :
			Scene(ctx),
			m_gui(window),
			m_window(window)
		{
			m_gui.setFont("resources/Butterpop.otf");

			auto panel = tgui::Panel::create();
			m_gui.add(panel);
			panel->setSize("50%", "40%");
			panel->setPosition("25%", "50%");
			panel->getRenderer()->setBackgroundColor({ 0,0,0,0 });

			auto button = tgui::Button::create("Play");
			panel->add(button);
			button->onClick([&]()
			{
				ctx.transition.start([&]()
				{
					api.comms.push("Platforming");
				});
			});
			button->setPosition(0, 0);
			button->setSize("100%", "25%");
			button->getRenderer()->setRoundedBorderRadius(16.f);

			button = tgui::Button::create("Level Editor");
			panel->add(button);
			button->onClick([&]()
			{
				ctx.transition.start([&]()
					{
						api.comms.push("LevelEditor");
					});
			});
			button->setPosition(0, "25%");
			button->setSize("100%", "25%");
			button->getRenderer()->setRoundedBorderRadius(16.f);

			button = tgui::Button::create("Settings");
			panel->add(button);
			button->onClick([&]()
			{
				ctx.transition.start([&]()
				{
					api.comms.push("Settings");
				});
			});
			button->setPosition(0, "50%");
			button->setSize("100%", "25%");
			button->getRenderer()->setRoundedBorderRadius(16.f);

			button = tgui::Button::create("Exit");
			panel->add(button);
			button->onClick([&]()
			{
				ctx.transition.start([&]()
				{
					m_window.close();
				});
			});
			button->setPosition(0, "75%");
			button->setSize("100%", "25%");
			button->getRenderer()->setRoundedBorderRadius(16.f);
		}

		void onEvent(const sf::Event& event) override
		{
			m_gui.handleEvent(event);
		}

		void draw(px::DrawCtx& ctx) const override
		{
			ctx.window.clear(sf::Color(0x222222ff));

			m_gui.draw();
		}

	private:

		mutable tgui::Gui m_gui;
		sf::Window& m_window;
	};
}