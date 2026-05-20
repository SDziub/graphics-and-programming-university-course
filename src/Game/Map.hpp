#pragma once

#include <cstdint>
#include <filesystem>
#include <unordered_map>

#include "pXL/pXL.hpp"

#include "Tile.hpp"

using Map = px::Grid<Tile>;

inline uint8_t getAdjacent(const Map& map, const sf::Vector2u position)
{
	uint8_t adjacent{};

	sf::Vector2u testPosition{ position.x, position.y - 1 };
	adjacent += 1 * (map.withinBounds(testPosition) && map.at(testPosition).tileName == map.at(position).tileName);
	testPosition = { position.x + 1, position.y };
	adjacent += 2 * (map.withinBounds(testPosition) && map.at(testPosition).tileName == map.at(position).tileName);
	testPosition = { position.x, position.y + 1 };
	adjacent += 4 * (map.withinBounds(testPosition) && map.at(testPosition).tileName == map.at(position).tileName);
	testPosition = { position.x - 1, position.y };
	adjacent += 8 * (map.withinBounds(testPosition) && map.at(testPosition).tileName == map.at(position).tileName);

	return adjacent;
}

void saveMap(const std::filesystem::path& path, const Map& map);
Map loadMap(const std::filesystem::path& path, const std::unordered_map<std::string, Tile>&);