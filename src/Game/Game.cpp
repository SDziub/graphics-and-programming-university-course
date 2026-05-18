#include <fstream>
#include <exception>

#include <nlohmann/json.hpp>

#include "Game.hpp"

namespace nl = nlohmann;

Game::Game()
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
	scenes.registerScene("Pause", [&]() { return std::make_unique<Scenes::Pause>(apiScene, window); });
	scenes.registerScene("Settings", [&]() { return std::make_unique<Scenes::Settings>(apiScene, mapping, window); });
	scenes.push("MainMenu");

	mapping.set("Jump", px::InputId::Space);
	mapping.set("Left", px::InputId::A);
	mapping.set("Right", px::InputId::D);
	mapping.set("Up", px::InputId::W);
	mapping.set("Down", px::InputId::S);
	mapping.set("Confirm", px::InputId::Space);
	mapping.set("Pause", px::InputId::Escape);

	m_ctx.tiles["empty"] = Tile{ Tile::Type::Air, "", "empty" };
	m_ctx.tiles["solid_block"] = Tile{ Tile::Type::Solid, "solid_block", "solid_block" };

	assets.tileSprites.set("solid_block", px::TileSprite{ "solid_block" });

	loadSprites();

	/*std::vector<px::AnimationFrame> idle{
		{{{ 0, 64  }, { 32, 32 }}, sf::milliseconds(300)},
		{{{ 32, 64  }, { 32, 32 }}, sf::milliseconds(300)}
	};

	std::vector<px::AnimationFrame> run;
	for (int32_t x = 0; x < 10; ++x)
	{
		run.push_back({ sf::IntRect({x * 32, 0}, {32, 32}), sf::milliseconds(75) });
	}

	std::vector<px::AnimationFrame> jump;
	for (int32_t x = 0; x < 5; ++x)
	{
		jump.push_back({ sf::IntRect({x * 32, 32}, {32, 32}), sf::milliseconds(75) });
	}

	std::vector<px::AnimationFrame> fall;
	for (int32_t x = 5; x < 7; ++x)
	{
		fall.push_back({ sf::IntRect({x * 32, 32}, {32, 32}), sf::milliseconds(100) });
	}

	px::AnimationClip idleClip(assets.textures.get("entities/player"), std::move(idle), px::AnimationClipType::Looped, { 16, 24 });
	px::AnimationClip runClip(assets.textures.get("entities/player"), std::move(run), px::AnimationClipType::Looped, { 16, 24 });
	px::AnimationClip jumpClip(assets.textures.get("entities/player"), std::move(jump), px::AnimationClipType::Sticky, { 16, 24 });
	px::AnimationClip fallClip(assets.textures.get("entities/player"), std::move(fall), px::AnimationClipType::Sticky, { 16, 24 });

	px::AnimationClipMap animations
	{
		{
			{"idle", std::move(idleClip)},
			{"run", std::move(runClip)},
			{"jump", std::move(jumpClip)},
			{"fall", std::move(fallClip)}
		},
		"idle"
	};

	assets.clipMaps.set("player", std::move(animations));

	std::vector<px::AnimationFrame> particle;
	for (int32_t x = 0; x < 4; ++x)
	{
		particle.push_back({ sf::IntRect{ {x * 4, 0},{4, 4} }, sf::milliseconds(50) });
	}

	px::AnimationClip particleClip(assets.textures.get("particle"), std::move(particle), px::AnimationClipType::Looped);

	px::AnimationClipMap particleAnim
	{
		{
			{"_", std::move(particleClip)}
		},
		"_"
	};

	assets.clipMaps.set("cloud_particle", std::move(particleAnim));

	std::vector<px::AnimationFrame> spike{ px::AnimationFrame{sf::IntRect{ {0, 0}, {16, 16} }} };

	px::AnimationClip spikeClip{
		assets.textures.get("spike"),
		std::move(spike),
		px::AnimationClipType::Looped,
		{8.0f, 16.0f}
	};

	px::AnimationClipMap spikeAnim
	{
		{{"_", std::move(spikeClip)}},
		"_"
	};

	assets.clipMaps.set("spike", std::move(spikeAnim));

	std::vector<px::AnimationFrame> platform{ px::AnimationFrame{sf::IntRect{ {0, 0}, {16, 16} }} };

	px::AnimationClip platformClip{
		assets.textures.get("platform"),
		std::move(platform),
		px::AnimationClipType::Looped,
		{8.0f, 8.0f}
	};

	px::AnimationClipMap platformAnim
	{
		{{"_", std::move(platformClip)}},
		"_"
	};

	assets.clipMaps.set("platform", std::move(platformAnim));*/

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
	player.emplace<Transform>(sf::Vector2f{ 0.0f, 0.0f }, sf::Vector2f{ 0.0f, 0.0f });
	player.emplace<Hitbox>(sf::FloatRect(
		sf::Vector2f(-0.25f, -0.25f),
		sf::Vector2f(0.5f, 0.75f)
	));
	player.emplace<Controllable>();
	player.emplace<px::Animation>(assets.clipMaps.get("player"));
	m_ctx.entities.set("player", std::move(player));

	EntityPrefab cloudParticle;
	cloudParticle.emplace<Transform>();
	cloudParticle.emplace<Lifetime>(sf::Time::Zero, sf::milliseconds(400));
	cloudParticle.emplace<IsParticle>();
	cloudParticle.emplace<px::Animation>(assets.clipMaps.get("particle"));
	m_ctx.entities.set("cloud_particle", std::move(cloudParticle));

	EntityPrefab spikePrefab;
	spikePrefab.emplace<Transform>();
	spikePrefab.emplace<Hitbox>(sf::FloatRect{ {-0.25f, -0.75f}, {0.5f, 0.75f} }, ColiderType::Hazard);
	spikePrefab.emplace<px::Animation>(assets.clipMaps.get("spike_up"));
	m_ctx.entities.set("spike", std::move(spikePrefab));

	EntityPrefab platformPrefab;
	platformPrefab.emplace<Transform>();
	platformPrefab.emplace<Hitbox>(sf::FloatRect{ { -0.5f, -0.5f}, {1.f,1.f} }, ColiderType::Platform);
	platformPrefab.emplace<px::Animation>(assets.clipMaps.get("platform"));
	m_ctx.entities.set("platform", std::move(platformPrefab));
}

void Game::loadSprites()
{
	recursiveLoad("resources/sprites", [&](const auto& path, const auto& name)
	{
		nl::json json;
		std::ifstream file(path);

		try
		{
			json << file;
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("{}", e.what());
			return;
		}

		const sf::Texture& texture = assets.textures.get(json["texture"]);
		sf::Vector2f origin{ static_cast<float>(json["origin"][0]), static_cast<float>(json["origin"][1]) };
		std::string defaultClip = json["default"];
		sf::Vector2i tile = static_cast<sf::Vector2i>(texture.getSize());
		tile.x /= json["grid"][0];
		tile.y /= json["grid"][1];

		px::AnimationClipMap clips;
		clips.fallback = defaultClip;
		for (const auto& [key, value] : json["clips"].items())
		{
			sf::Time time = sf::seconds(value["duration"]);

			std::string typeString = value["type"];
			px::AnimationClipType type = px::AnimationClipType::Normal;
			if (typeString == "looped")
			{
				type = px::AnimationClipType::Looped;
			}
			if (typeString == "sticky")
			{
				type = px::AnimationClipType::Sticky;
			}

			std::vector<px::AnimationFrame> frames;
			for (const auto& vector : value["frames"])
			{
				sf::IntRect frameRect{ {vector[0] * tile.x, vector[1] * tile.y}, tile };
				frames.push_back({ frameRect, time });
			}

			px::AnimationClip clip{ texture };
			clip.frames = std::move(frames);
			clip.origin = origin;
			clip.type = type;

			clips.map.insert({ key, std::move(clip)});
		}

		assets.clipMaps.set(name, std::move(clips));
	});
}