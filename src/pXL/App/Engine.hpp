#pragma once

#include <stdint.h>
#include <algorithm>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "SceneStack.hpp"
#include "Transition.hpp"
#include "Input.hpp"
#include "Scaling.hpp"

namespace px
{
	class Engine
	{
	public:

		void run();

	protected:

		Engine();
		virtual ~Engine();

		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;

		virtual void interceptEvent(const sf::Event& event) {}

		virtual void preEvent() {}
		virtual void postEventPreUpdate() {}
		virtual void postUpdatePreDraw() {}
		virtual void postDraw() {}

		void recursiveLoad(const std::string& directoryPath,
			std::function<void(const std::filesystem::path& path,
				const std::string& name)>&& call);

		struct ScaleSettings
		{
			sf::Vector2f minimumUnits{ 20.0f, 10.25f };
			uint32_t pixelsPerUnit{ 16 };
		};

		Assets assets;
		sf::RenderWindow window;
		SceneStack scenes;
		Input frameInput;
		Input tickInput;
		Mapping mapping{ frameInput };
		Transition transition;
		Scaling scaling;

		sf::Time elapsed{};

		EngineApi engApi{
			scenes,
			assets,
			mapping,
			scaling
		};

		SceneInitCtx apiScene{
			scenes,
			transition,
			engApi
		};

		bool showFps{};

	private:

		static constexpr uint32_t k_tps{ 60 };
		static constexpr sf::Time k_fixedDt = sf::microseconds(1000000 / k_tps);
	};

	inline Engine::Engine() :
		window(sf::VideoMode(sf::Vector2u{ 1280,720 }), "Game", sf::Style::Default),
		scaling(window)
	{
		window.setKeyRepeatEnabled(false);
		ImGui::SFML::Init(window);
		ImGui::GetIO().FontGlobalScale = 2.0f;

		scenes.setOnChangeCallback([&]() {
			frameInput.newUpdate();
			tickInput.newUpdate();
		});

		window.setMinimumSize(scaling.getMinimumWindowSize());
	}

	inline Engine::~Engine()
	{
		ImGui::SFML::Shutdown();
	}

	inline void Engine::run()
	{
		sf::Clock clock;
		sf::Time acumulator;

		while (window.isOpen())
		{
			scenes.flush();

			preEvent();

			frameInput.newUpdate();

			while (const auto event = window.pollEvent())
			{
				frameInput.readEvent(*event);
				tickInput.readEvent(*event);

				ImGui::SFML::ProcessEvent(window, *event);

				interceptEvent(*event);

				if (event->is<sf::Event::Closed>())
				{
					window.close();
				}
			}

			scaling.update();

			postEventPreUpdate();

			sf::Time realDt = clock.restart();
			elapsed += realDt;

			ImGui::SFML::Update(window, realDt);

			acumulator = std::min(acumulator + realDt, k_fixedDt * 4.001f);

			mapping.setUnderlyingInput(tickInput);

			UpdateCtx fixedUpdateCtx{
				window,
				k_fixedDt,
				transition
			};

			while (acumulator >= k_fixedDt)
			{
				scenes.fixedUpdate(fixedUpdateCtx);
				tickInput.newUpdate();
				acumulator -= k_fixedDt;
			}

			mapping.setUnderlyingInput(frameInput);

			transition.update(realDt.asSeconds());

			UpdateCtx updateCtx{
				window,
				realDt,
				transition
			};

			scenes.update(updateCtx);

			postUpdatePreDraw();

			float alpha = acumulator / k_fixedDt;

			DrawCtx drawCtx{
				window,
				assets,
				alpha
			};

			window.clear(sf::Color::Black);

			scenes.draw(drawCtx);

			ImGui::SFML::Render(window);

			window.draw(transition);

			if (showFps)
			{
				sf::Text fpsDisplay(assets.font, std::to_string(1.f / realDt.asSeconds()));
				window.draw(fpsDisplay);
			}

			postDraw();

			window.display();
		}
	}

	inline void Engine::recursiveLoad(const std::string& directoryPath, std::function<void(const std::filesystem::path& path, const std::string& name)>&& call)
	{
		if (!std::filesystem::exists(directoryPath))
		{
			return;
		}

		for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}

			std::string name = std::filesystem::relative(entry.path(), directoryPath).replace_extension().generic_string();

			call(entry.path(), name);
		}
	}
}