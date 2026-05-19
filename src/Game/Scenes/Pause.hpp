#pragma once

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "pXL/pXL.hpp"

namespace Scenes
{
	class Pause : public px::Scene
	{
	public:

		Pause(px::SceneInitCtx& ctx, sf::RenderWindow& window) :
			px::Scene(ctx),
			m_gui(window)
		{
			ctx.properties.setTransparency(true);

			m_gui.setFont("resources/Butterpop.otf");

			auto panel = tgui::Panel::create();
			m_gui.add(panel);
			panel->setSize("50%", "40%");
			panel->setPosition("25%", "30%");
			panel->getRenderer()->setBackgroundColor({ 0,0,0,0 });

			auto label = tgui::Label::create("Pause Menu");
			panel->add(label);
			label->setPosition(0, 0);
			label->setSize("100%", "25%");
			label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
			label->setVerticalAlignment(tgui::VerticalAlignment::Center);

			auto button = tgui::Button::create("Resume");
			panel->add(button);
			button->onClick([&]()
			{
				api.comms.pop();
			});
			button->setPosition(0, "25%");
			button->setSize("100%", "25%");
			button->getRenderer()->setRoundedBorderRadius(16.f);

			button = tgui::Button::create("Settings");
			panel->add(button);
			button->onClick([&]()
			{
				api.comms.push("Settings");
			});
			button->setPosition(0, "50%");
			button->setSize("100%", "25%");
			button->getRenderer()->setRoundedBorderRadius(16.f);

			button = tgui::Button::create("Back");
			panel->add(button);
			button->onClick([&]()
			{
				api.comms.popUntil("MainMenu");
			});
			button->setPosition(0, "75%");
			button->setSize("100%", "25%");
			button->getRenderer()->setRoundedBorderRadius(16.f);
		}

		void onEvent(const sf::Event& event)
		{
			m_gui.handleEvent(event);
		}

		void draw(px::DrawCtx& ctx) const override
		{
			sf::RectangleShape darkRect(static_cast<sf::Vector2f>(ctx.window.getSize()));
			darkRect.setFillColor(sf::Color(0, 0, 0, 100));
			ctx.window.draw(darkRect);

			m_gui.draw();
		}

	private:

		mutable tgui::Gui m_gui;
	};
}