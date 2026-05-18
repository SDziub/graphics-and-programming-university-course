#pragma once

#include <iostream>

#include <spdlog/spdlog.h>

#include "pXL/pXL.hpp"

#include "Game/Context.hpp"
#include "Scenes/Scenes.hpp"

class Game : public px::Engine
{
public:

	Game();

private:

	void loadSprites();

	Context m_ctx;
};