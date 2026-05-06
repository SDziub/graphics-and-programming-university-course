#pragma once

#include <string>

#include <SFML/System.hpp>

struct Hitbox
{
	sf::FloatRect rect{};
};

struct Transform
{
	sf::Vector2f pos{};
	sf::Vector2f vel{};
	sf::Vector2f oldPos{};
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

enum class ColiderType
{
	Killing,
	Pushable,
	Static,
	Dynamic
};

struct Colider
{
	ColiderType type;
};

struct EntityType
{
	std::string name;
};