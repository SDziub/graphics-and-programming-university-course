#pragma once

#include "pXL/pXL.hpp"

#include "Tile.hpp"

using TileMap = px::Grid<const Tile*>;

inline const TileMap TileMapEmpty({ 0, 0 }, nullptr);