#pragma once

#include <unordered_map>

#include "pXL/pXL.hpp"

#include "Tile.hpp"
#include "EntityPrefab.hpp"

struct Context
{
	px::Registry<EntityPrefab> entities;
	std::unordered_map<std::string, Tile> tiles;

	std::vector<std::string> maps = {
			"./resources/maps/1.json",
			"./resources/maps/2.json",
			"./resources/maps/3.json",
			"./resources/maps/4.json",
			"./resources/maps/5.json",
			"./resources/maps/6.json",

	};
};