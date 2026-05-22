#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "pXL/pXL.hpp"

namespace Scenes
{
	class Settings : public px::Scene
	{
	public:

		Settings(px::SceneInitCtx& ctx, px::Mapping& mapping, sf::RenderWindow& window) :
			px::Scene(ctx),
			m_gui(window),
			m_mapping(mapping)
		{
			m_gui.setFont("resources/Butterpop.otf");

			/*uint32_t y{};
			auto setButton = [&](std::string_view name, std::string& action)
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
				};*/

			auto scrollablePanel = tgui::ScrollablePanel::create({ "50%", "70%" });
			scrollablePanel->setPosition("25%", "20%");
			scrollablePanel->getRenderer()->setBackgroundColor({ 0,0,0,0 });
			m_gui.add(scrollablePanel);
			
			float y = 0;
			const float widgetY = 50.f;
			const float gapY = 5.f;

			auto keybinds = m_mapping.data();



			for (const auto& [action, key] : m_mapping.data())
			{
				auto label = tgui::Label::create(action);
				label->setPosition(0, y);
				label->setSize("50%", widgetY);
				label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
				label->setVerticalAlignment(tgui::VerticalAlignment::Center);
				scrollablePanel->add(label);
				auto button = tgui::Button::create();
				button->setPosition("50%", y);
				y += widgetY + gapY;
				button->setSize("50%", widgetY);
				button->setText(px::stringifyInputId(key));
				button->onPress([=, this]()
				{
					this->m_rebinding = action;
					button->setText("rebinding");
				});
				button->getRenderer()->setRoundedBorderRadius(16.f);
				scrollablePanel->add(button, action);
			}
		}

		void onEvent(const sf::Event& event) override
		{
			if (m_rebinding && event.is<sf::Event::MouseButtonPressed>())
			{
				return;
			}

			m_gui.handleEvent(event);
		}

		void update(px::UpdateCtx& ctx) override
		{
			if (!m_rebinding)
			{
				if (api.mapping.isPressed(px::InputId::Escape))
				{
					api.comms.pop();
				}

				return;
			}

			if (api.mapping.isPressed(px::InputId::Escape))
			{
				m_gui.get<tgui::Button>(*m_rebinding)->setText(px::stringifyInputId(m_mapping.data().at(*m_rebinding)));
				m_rebinding.reset();
				return;
			}

			if (const auto pressed = api.mapping.getJustPressed())
			{
				m_gui.get<tgui::Button>(m_rebinding.value())->setText(px::stringifyInputId(*pressed));
				m_mapping.set(m_rebinding.value(), *pressed);
				m_rebinding.reset();
			}
		}

		void draw(px::DrawCtx& ctx) const override
		{
			m_gui.draw();
		}

	private:

		mutable tgui::Gui m_gui;
		std::optional<std::string> m_rebinding;
		px::Mapping& m_mapping;
	};
}