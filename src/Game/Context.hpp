#pragma once

#include <unordered_map>

#include "pXL/pXL.hpp"

#include "Tile.hpp"
#include "EntityPrefab.hpp"

struct Context
{
	px::Registry<EntityPrefab> entities;
	std::unordered_map<std::string, Tile> tiles;
};