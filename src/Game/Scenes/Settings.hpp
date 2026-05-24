#pragma once

#include <optional>
#include <string>
#include <fstream>

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <nlohmann/json.hpp>

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
			m_gui.setFont(RESOURCES "Butterpop.otf");

			uint32_t y{};
			auto setButton = [&](tgui::Button::Ptr button, const std::string& action)
				{
					button->setPosition("(parent.width - width) / 2 + 200", ("parent.height / 2 - 180 + " + std::to_string(y)).c_str());
					button->setSize("200", "50");

					auto renderer = button->getRenderer();
					renderer->setRoundedBorderRadius(40);
					renderer->setBorderColor({ 107, 62, 117 });
					renderer->setBackgroundColor({ 234, 173, 237 });
					renderer->setBackgroundColorDown({ 143, 211, 255 });
					renderer->setBackgroundColorHover({ 168, 132, 243 });

					auto label = tgui::Label::create(action.c_str());
					label->setPosition("(parent.width - width) / 2 - 200", ("parent.height / 2 - 180 + " + std::to_string(y)).c_str());
					label->setSize("200", "50");;
					label->getRenderer()->setTextColor(tgui::Color::White);
					label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
					label->setVerticalAlignment(tgui::VerticalAlignment::Center);

					m_gui.add(button, action);
					m_gui.add(label);

					y += 60;
				};

			auto label = tgui::Label::create("Pause Menu");
			m_gui.add(label);
			label->setPosition("(parent.width - width) / 2", ("parent.height / 2 - 180 + " + std::to_string(y)).c_str());
			y += 60;
			label->setSize("200", "50");;
			label->getRenderer()->setTextColor(tgui::Color::White);
			label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
			label->setVerticalAlignment(tgui::VerticalAlignment::Center);

			label = tgui::Label::create("Master Volume");
			m_gui.add(label);
			label->setPosition("(parent.width - width) / 2 - 200", ("parent.height / 2 - 180 + " + std::to_string(y)).c_str());
			label->setSize("200", "50");;
			label->getRenderer()->setTextColor(tgui::Color::White);
			label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
			label->setVerticalAlignment(tgui::VerticalAlignment::Center);

			auto slider = tgui::Slider::create(0, 100);
			slider->setValue(sf::Listener::getGlobalVolume());
			slider->onValueChange([](float value)
				{
					sf::Listener::setGlobalVolume(value);
				});
			m_gui.add(slider);
			slider->setPosition("(parent.width - width) / 2 + 200", ("parent.height / 2 - 180 + 20 + " + std::to_string(y)).c_str());
			slider->setSize("200", "10");

			auto renderer = slider->getRenderer();
			renderer->setBorderColor({ 107, 62, 117 });
			renderer->setTrackColor({ 234, 173, 237 });
			renderer->setTrackColorHover({ 168, 132, 243 });

			y += 60;

			for (const auto& [action, key] : m_mapping.data())
			{
				auto button = tgui::Button::create(px::stringifyInputId(key));
				button->onPress([=, this]()
				{
					this->m_rebinding = action;
					button->setText("Rebinding (ECS to cancel)");
				});
				setButton(button, action);
			}
		}

		~Settings()
		{
			saveSettings();
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
					api.transition.start([&]()
						{
							api.comms.pop();
						});
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
			ctx.window.draw(px::Background(api.assets.backgrounds.get("settings"), 0.f));

			ctx.window.setView(px::getRenderTargetView(ctx.window));
			sf::RectangleShape vignette(static_cast<sf::Vector2f>(ctx.window.getSize()));
			vignette.setTexture(&api.assets.textures.get("vignette"));
			vignette.setFillColor({ 0, 0, 0, 150 });
			ctx.window.draw(vignette);

			m_gui.draw();
		}

	private:

		void saveSettings() const
		{
			nlohmann::json json;
			
			json["volume"] = sf::Listener::getGlobalVolume();

			std::unordered_map<std::string, std::string> actions;
			for (const auto& [action, key] : m_mapping.data())
			{
				actions.insert({ action, px::stringifyInputId(key) });
			}

			json["actions"] = actions;

			std::ofstream file("settings.json");
			file << json;
		}

		mutable tgui::Gui m_gui;
		std::optional<std::string> m_rebinding;
		px::Mapping& m_mapping;
	};
}