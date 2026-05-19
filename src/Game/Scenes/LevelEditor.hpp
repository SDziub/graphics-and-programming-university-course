#pragma once

#include "pXL/pXL.hpp"

#include "Game/Context.hpp"
#include "Game/Tile.hpp"
#include "Game/World.hpp"
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
		enum selected
		{
			entity = 0,
			tile = 1
		};

		//window / map config
		int windowSize = 720;
		int mapWidth = 25;
		int mapHeight = 25;

		World LE_world;
		Context& m_ctx;
		px::Grid<Tile>& LE_map;
		entt::registry& LE_registry;
		entt::entity player;
		std::vector<std::string> entities;
		
		
		
		sf::Vector2i lastMousePos;
		sf::Vector2i viewPosition{0,0};
		
		//map layout
		std::unordered_map<std::string, Tile> LE_tiles;
		std::unordered_map<std::string, std::pair<int, int>> LE_Entities;
				
		int selectState = selected::tile;
		//selectable
		int currentTile = 0;
		int currentMap = 1;
		int currentEntity = 1;
		
		//player & entity
		sf::Rect<float>playerRect;
		bool isDragging = false;
		sf::Vector2i dragStartPos;
		const int dragThreshold = 4;

		//tileSaving
		std::vector<std::string> TileName;
		std::vector<const char*> TileNameC;
		
		//saveLoad stuff
		std::string newMapName;
		std::string currMapName = std::to_string(currentMap);
		std::string mapPath = "./resources/maps/" + currMapName + ".json";
		std::string mapPathS = "./resources/maps/" + newMapName + ".json";


	};
}