#pragma once

#include <memory>

#include <entt/entt.hpp>

#include "TileMap.hpp"
#include "Device.hpp"

struct Level
{
	TileMap map;
	entt::registry entities;
	std::vector<std::unique_ptr<Device>> devices;
};