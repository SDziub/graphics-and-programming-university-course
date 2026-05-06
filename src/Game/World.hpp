#pragma once

#include <entt/entt.hpp>

#include "Map.hpp"

struct World
{
	entt::registry entities;
	Map map;
};