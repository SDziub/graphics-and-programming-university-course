#pragma once

#include <string>
#include <cstdint>

#include <SFML/System.hpp>

struct Hitbox
{
	sf::FloatRect rect{};
	sf::FloatRect precomputed{};
};

enum class ColiderType : uint8_t
{
	Solid,
	Platform,
	Hazard
};

struct Colider
{
	ColiderType type{};
	bool dynamic{};
};

enum class Facing : uint8_t
{
	Left,
	Right
};

struct Transform
{
	sf::Vector2f pos{};
	sf::Vector2f vel{};
	sf::Vector2f oldPos{};
	float jumpStartY{};
	Facing facing{};
};

struct Controllable
{
	sf::Time cayoteTime{};
	sf::Time jumpBuffer{};
	bool canJump{};
	bool grounded{};
};

struct Lifetime
{
	sf::Time lived;
	sf::Time max;
};

struct EntityType
{
	std::string name;
};

struct IsParticle
{

};