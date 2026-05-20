#pragma once

#include <iostream>

#include <spdlog/spdlog.h>

#include "pXL/pXL.hpp"

#include "Game/Context.hpp"
#include "Scenes/Scenes.hpp"

class Game : public px::Client
{
public:

	Game()
	{
		//window.setFramerateLimit(60);

		recursiveLoad("resources/textures", [&](const auto& path, const auto& name) {
			sf::Texture texture;
			if (!texture.loadFromFile(path))
			{
				return;
			}
			texture.setRepeated(true);

			assets.textures.set(name, std::move(texture));

			SPDLOG_INFO("Texture loaded: {}", name);
		});

		scenes.registerScene("MainMenu", [&]() { return std::make_unique<Scenes::MainMenu>(apiScene, window); });
		scenes.registerScene("LevelEditor", [&]() { return std::make_unique<Scenes::LevelEditor>(apiScene, m_ctx); });
		scenes.registerScene("Platforming", [&]() { return std::make_unique<Scenes::Platforming>(apiScene, m_ctx); });
		scenes.registerScene("Pause", [&]() {return std::make_unique<Scenes::Pause>(apiScene); });
		scenes.push("MainMenu");

		mapping.set("Jump", px::InputId::Space);
		mapping.set("Left", px::InputId::A);
		mapping.set("Right", px::InputId::D);
		mapping.set("Up", px::InputId::W);
		mapping.set("Down", px::InputId::S);
		mapping.set("Confirm", px::InputId::Space);
		mapping.set("Pause", px::InputId::Escape);

		m_ctx.tiles["empty"] = Tile{Tile::Type::Air, "", "empty"};
		m_ctx.tiles["solid_block"] = Tile{ Tile::Type::Solid, "solid_block", "solid_block"};

		assets.tileSprites.set("solid_block", px::TileSprite{ "solid_block" });

		std::vector<px::Frame> idle{ {{{ 0, 32  }, { 32, 32 }}, sf::milliseconds(200)} };

		std::vector<px::Frame> run;
		for (int32_t x = 0; x < 10; ++x)
		{
			run.push_back({ sf::IntRect({x * 32, 0}, {32, 32}), sf::milliseconds(75) });
		}

		px::Clip idleClip(std::move(idle));
		idleClip.setLooping(true);
		px::Clip runClip(std::move(run));
		runClip.setLooping(true);

		px::SpriteData animations(assets.textures.get("entities/player"));
		animations.setClip("idle", std::move(idleClip));
		animations.setClip("run", std::move(runClip));

		assets.sprites.set("knight", std::move(animations));

		px::BackgroundData background(
			{
				{ assets.textures.get("background/0"), 0.03125f },
				{ assets.textures.get("background/1"), 0.0625f },
				{ assets.textures.get("background/2"), 0.125f },
				{ assets.textures.get("background/4"), 0.25f },
				{ assets.textures.get("background/3"), 0.5f },
				{ assets.textures.get("background/5"), 1.0f }
			}
		);

		assets.backgrounds.set("background", std::move(background));

		assets.font = sf::Font("resources/Butterpop.otf");

		EntityPrefab player;
		player.emplace<Transform>(sf::Vector2f(3.5f, 3.5f), sf::Vector2f(0.0f, 0.0f));
		player.emplace<Hitbox>(sf::Rect<float>(
			sf::Vector2f(-0.25f, -0.25f),
			sf::Vector2f(0.5f, 0.75f)
		));
		player.emplace<Controllable>();
		m_ctx.entities.set("player", std::move(player));
	}

	void preEvent() override
	{
		if (frameInput.isPressed(px::InputId::F))
		{
			showFps = !showFps;
		}
	}

private:

	Context m_ctx;
};