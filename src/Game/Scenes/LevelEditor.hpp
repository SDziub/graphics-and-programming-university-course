#pragma once

#include "pXL/pXL.hpp"

#include "Game/Context.hpp"
#include "Game/Tile.hpp"
#include <filesystem>

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
		entt::registry LE_registry;
		entt::entity player;
		std::vector<entt::entity> entities;
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

		


		sf::Rect<float>playerRect;
		
		std::vector<std::string> TileName;
		std::vector<const char*> TileNameC;

		std::string newMapName;
		std::string currMapName = std::to_string(currentMap);
		std::string mapPath = "./resources/maps/" + currMapName + ".json";
		std::string mapPathS = "./resources/maps/" + newMapName + ".json";


	};
}