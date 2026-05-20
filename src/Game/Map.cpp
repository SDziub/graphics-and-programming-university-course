#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

#include <iostream>

#include "Map.hpp"

namespace nl = nlohmann;

void saveMap(const std::filesystem::path& path, const Map& map)
{
	(std::filesystem::exists(path)) 
		? SPDLOG_INFO("Saved the map to existing file: {}", path.string())
	: SPDLOG_INFO("Created new save file: {}", path.string())
		;
	
		nl::json mapSaveJ;
		mapSaveJ["width"] = map.size().x;
		mapSaveJ["height"] = map.size().y;
		nl::json rows = nl::json::array();

		for (uint32_t y = 0; y < map.size().y; y++) {
			nl::json row = nl::json::array();
			for (uint32_t x = 0; x < map.size().x; x++) {
				row.push_back(map.at({ x,y }).tileName);
			}
			rows.push_back(row);
		}
		mapSaveJ["tiles"] = rows;

		std::ofstream mapSave(path);
		mapSave << mapSaveJ.dump(4);
		mapSave.close();	
}

Map loadMap(const std::filesystem::path& path, const std::unordered_map<std::string, Tile>&  tiles)
{	

	std::ifstream loadedMap(path);
	if (!loadedMap.is_open()) {
		SPDLOG_ERROR("Failed to load the map from {}", path.string());
		Map failedMap({ 1,1 }, tiles.at("grass"));
		return failedMap;
	}

	nl::json loadedMapJson = nl::json::parse(loadedMap);
	uint32_t width = loadedMapJson["width"];
	uint32_t height = loadedMapJson["height"];

	std::string baseTile = "dirt";

	sf::Vector2u size = { width, height };
	Map map(size, tiles.at("empty")); 
	
	
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			std::string loadedTileName = loadedMapJson["tiles"][y][x];
			if (tiles.contains(loadedTileName)) {
				map.at({ x, y }) = tiles.at(loadedTileName);
			}
			else {
				SPDLOG_INFO("Given '{}' tile doesnt exist", loadedTileName);

			}
		}
	}
	SPDLOG_INFO("Loaded the map from {}", path.string());

	/*for (uint32_t y = 0;y< height; ++y) {
		for (uint32_t x = 0;x< width; ++x) {
			std::cout << map.at({ x,y }).tileName;
		}
		std::cout << std::endl;
	}*/
	return map;
}