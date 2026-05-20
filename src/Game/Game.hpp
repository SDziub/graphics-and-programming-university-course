#pragma once

#include <iostream>
#include <functional>
#include <unordered_map>
#include <string>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <SFML/Audio.hpp>

#include "pXL/pXL.hpp"

#include "Game/Context.hpp"
#include "Scenes/Scenes.hpp"

class Game : public px::Engine
{
public:

	Game();

private:

	void initPrefabGenerators();
	void loadSprites();

	using PrefabGenerator = std::function<EntityPrefab(const nlohmann::json& obj)>;

	Context m_ctx;
	std::unordered_map<std::string, PrefabGenerator> m_prefabGenerators;

	sf::Music m_music;
};