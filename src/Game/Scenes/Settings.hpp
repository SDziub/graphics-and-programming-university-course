#pragma once

#include <optional>
#include <string>

#include <SFML/Graphics.hpp>
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

			auto scrollablePanel = tgui::ScrollablePanel::create({ "50%", "70%" });
			scrollablePanel->setPosition("25%", "20%");
			scrollablePanel->getRenderer()->setBackgroundColor({ 0,0,0,0 });
			m_gui.add(scrollablePanel);
			
			float y = 0;
			const float widgetY = 50.f;
			const float gapY = 5.f;

			for (const auto& [action, key] : m_mapping.data())
			{
				auto label = tgui::Label::create(action);
				label->setPosition(0, y);
				label->setSize("50%", widgetY);
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
				button->getRenderer()->setRoundedBorderRadius(8.f);
				scrollablePanel->add(button, "keybind_" + action);
			}
		}

		void onEvent(const sf::Event& event) override
		{
			if (api.mapping.isPressed(px::InputId::Escape))
			{
				api.comms.pop();
				return;
			}

			if (m_rebinding)
			{
				return;
			}

			m_gui.handleEvent(event);
		}

		void update(px::UpdateCtx& ctx) override
		{
			if (!m_rebinding)
			{
				return;
			}

			if (const auto pressed = api.mapping.getJustPressed())
			{
				m_gui.get<tgui::Button>("keybind_" + m_rebinding.value())->setText(px::stringifyInputId(*pressed));
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