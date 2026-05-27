#pragma once

#include <vector>
#include <cstdint>

#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include "Components.hpp"

enum class DeprecatedDeviceType : uint8_t
{
	And, Or, Timed
};

struct DeprecatedDeviceIO
{
	entt::entity entity;
	bool inverted{};
};

struct DeprecatedDevice
{
	DeprecatedDevice(DeprecatedDeviceType type = DeprecatedDeviceType::And) :
		type(type)
	{}

	std::vector<DeprecatedDeviceIO> in, out;
	sf::Time onTime, offTime, accumulated;
	DeprecatedDeviceType type;
};