#pragma once

struct Hitbox
{
	sf::Rect<float> rect{};
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