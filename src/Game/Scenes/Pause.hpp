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

			auto label = tgui::Label::create("Pause Menu");
			m_gui.add(label);
			label->setPosition("(parent.width - width) / 2", ("parent.height / 2 - 120 + " + std::to_string(y)).c_str());
			y += 60;
			label->setSize("200", "50");;
			label->getRenderer()->setTextColor(tgui::Color::White);
			label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
			label->setVerticalAlignment(tgui::VerticalAlignment::Center);

			auto button = tgui::Button::create("Resume");
			button->onClick([&]()
				{
					api.comms.pop();
				});
			setButton(button);

			button = tgui::Button::create("Settings");
			button->onClick([&]()
				{
					api.comms.push("Settings");
				});
			setButton(button);

			button = tgui::Button::create("Back");
			button->onClick([&]()
				{
					api.comms.popUntil("MainMenu");
				});
			setButton(button);
		}

		void onEvent(const sf::Event& event) override
		{
			m_gui.handleEvent(event);
		}

		void update(px::UpdateCtx& ctx) override
		{
			if (api.mapping.isPressed(px::InputId::Escape))
			{
				api.comms.pop();
			}
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