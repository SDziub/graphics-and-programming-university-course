#pragma once

#include <SFML/System.hpp>

constexpr float k_acceleration = 25.0f;
constexpr float k_deceleration = 35.0f;
constexpr float k_maxSpeed = 6.0f;
constexpr float k_jumpVelocity = 13.25f;
constexpr float k_downAcceleration = 30.0f;
constexpr float k_maxDownAcceleration = 20.0f;
constexpr float k_fallMultiplayer = 1.5f;
constexpr sf::Time k_bufferedJumpLimit = sf::milliseconds(150);
constexpr sf::Time k_cayoteTime = sf::milliseconds(150);