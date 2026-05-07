#pragma once

#include <SFML/Graphics.hpp>

#include "pXL/pXL.hpp"

class Camera
{
public:

	// todo: implement
	Camera() = delete;

private:

	sf::View m_view;
	px::Scaling m_scaling;
};