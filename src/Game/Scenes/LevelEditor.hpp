#pragma once

#include "pXL/pXL.hpp"

#include "Game/Context.hpp"
#include "Game/Tile.hpp"

namespace Scenes
{
	class LevelEditor : public px::Scene
	{
	public:
			
		LevelEditor(px::SceneInitCtx ctx, Context& gctx);

		void update(px::UpdateCtx& ctx) override;
		void draw(px::DrawCtx& ctx) const override;
		void resizeMap();
	private:
		Context& m_ctx;
		sf::Vector2i lastMousePos;
		sf::Vector2i viewPosition{0,0};
		std::unordered_map<std::string, Tile> LE_tiles;
		int windowSize = 720;
		int mapWidth = 25;
		int mapHeight = 25;
		px::Grid<Tile> LE_map;
		int currentTile = 0;
		int currentMap = 1;
		std::vector<std::string> maps = {
			"./resources/maps/1.json",
			"./resources/maps/2.json",
			"./resources/maps/3.json",
			"./resources/maps/4.json",
			"./resources/maps/5.json",
			"./resources/maps/6.json",

		};
		std::vector<std::string> TileName;
		std::vector<const char*> TileNameC;
		std::string mapName = "./resources/maps/" + std::to_string(currentMap) + ".json";

	};
}