#include "LevelEditor.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Game/Map.hpp"
#include <spdlog/spdlog.h>

/*
	poruszanie graczem myszka
	
*/

Scenes::LevelEditor::LevelEditor(px::SceneInitCtx ctx, Context& gctx) :
	Scene(ctx),
	m_ctx(gctx),
	LE_map(sf::Vector2u(mapWidth, mapHeight), m_ctx.tiles["empty"]) 
{
	
	for (const auto& [tilename, _ ] : m_ctx.tiles) {
		TileName.push_back(tilename);
	}

	//for (const auto& [entityName, _] : m_ctx.entities) { }

	//spawning entity
	player = m_ctx.entities.get("player").spawn(LE_registry);
	auto pp = LE_registry.get<Transform>(player);
	SPDLOG_INFO("Spawned player ar {},{}", pp.pos.x, pp.pos.y);
	LE_tiles = m_ctx.tiles;

	lastMousePos = api.mapping.getMousePosition();
	
}



void Scenes::LevelEditor::update(px::UpdateCtx& ctx)
{
	if (ImGui::Begin("Editor Menu"))
	{
		if (ImGui::Button("Back"))
		{
			api.comms.pop({});
		}

		static int prevWidth = mapWidth;
		static int prevHeight = mapHeight;

		ImGui::DragInt("##mapheight", &mapHeight, 0.5f,25,100); //Set map Height
		if (ImGui::IsItemDeactivatedAfterEdit) {
			resizeMap();
		}
		ImGui::DragInt("##mapWidth", &mapWidth, 0.5f, 25, 100); //Set map Width
		if (ImGui::IsItemDeactivatedAfterEdit) {
			resizeMap();
		}
		
		
		static bool item_highlight = false;
		int item_highlighted_idx = -1;
		if (ImGui::BeginListBox("Choose tile")) //Choose tile list
		{
			for (int n = 0; n < TileName.size(); n++)
			{
				const bool is_selected = (currentTile == n);
				if (ImGui::Selectable(TileName[n].c_str(), is_selected))
					currentTile = n;
				if (item_highlight && ImGui::IsItemHovered())
					item_highlighted_idx = n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			
			ImGui::EndListBox();
			
			if (ImGui::BeginListBox("Choose map")) //Choose map list
			{

				for (int n = 0; n < m_ctx.maps.size(); n++)
				{
					const bool is_selected = (currentMap == n);
					if (ImGui::Selectable(m_ctx.maps[n].c_str(), is_selected))
						currentMap = n;
						mapPath = m_ctx.maps[currentMap];
					if (item_highlight && ImGui::IsItemHovered())
						item_highlighted_idx = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}

			

			//Input Map Name
			static char buf5[255] = ""; ImGui::InputText("no blank", buf5, sizeof(buf5), ImGuiInputTextFlags_CharsNoBlank);
			
				if (ImGui::Button("New")) {
					newMapName = buf5;
					mapPathS = "./resources/maps/" + newMapName + ".json";
					saveMap(mapPathS, LE_map);
					m_ctx.maps.push_back(mapPathS);
				}ImGui::SameLine();

				if (ImGui::Button("Save")) {
					saveMap(mapPath, LE_map);
				}ImGui::SameLine();

				if (ImGui::Button("Load")) {
					LE_map = loadMap(mapPath, LE_tiles);
					SPDLOG_INFO("Map size after load: {}x{}", LE_map.size().x, LE_map.size().y);
					SPDLOG_INFO("Tile [0,0]: {}", LE_map.at({ 0,0 }).tileName);
				}
		}
		ImGui::End();
	}

	sf::Rect<int> rect{
		sf::Vector2i(0,0), sf::Vector2i((windowSize / mapHeight),(windowSize / mapHeight))
	};

	const int tileSize = windowSize / mapHeight;

	sf::Vector2i mousePosition = api.mapping.getMousePosition();
	mousePosition += viewPosition;

	sf::Vector2i currMousePos = api.mapping.getMousePosition();
	sf::Vector2f worldMousePos = sf::Vector2f(currMousePos + viewPosition) / float(tileSize);
	auto playerTransform = LE_registry.get<Transform>(player);
	auto playerHitbox = LE_registry.get<Hitbox>(player).rect;
	
	playerHitbox.position += playerTransform.pos;
	playerRect = playerHitbox;

	bool isDraggingPlayer = playerHitbox.contains(worldMousePos);
	

	if (!ImGui::GetIO().WantCaptureMouse)
	{
		sf::Vector2i tileCoord = mousePosition / tileSize;

		bool inBounds = tileCoord.x >= 0 && tileCoord.y >= 0
			&& static_cast<unsigned>(tileCoord.x) < mapWidth
			&& static_cast<unsigned>(tileCoord.y) < mapHeight;

		if (api.mapping.isHeld(px::InputId::MLeft) && inBounds && !isDraggingPlayer) {
				sf::Vector2u MP = static_cast<sf::Vector2u>(tileCoord);
				LE_map.at(MP) = m_ctx.tiles.at(TileName[currentTile]);
				
		}
		if (api.mapping.isHeld(px::InputId::MLeft) && inBounds && isDraggingPlayer) {
			sf::Vector2i offset = api.mapping.getMouseDelta();
			auto& transform = LE_registry.get<Transform>(player);
			transform.pos += sf::Vector2f(offset) / float(tileSize);
		}
			
	}
	

	if (api.mapping.isHeld(px::InputId::MRight)) {
		sf::Vector2i mouseDiff = lastMousePos - currMousePos;
		viewPosition += mouseDiff;
	}
	lastMousePos = currMousePos;
}

void Scenes::LevelEditor::draw(px::DrawCtx& ctx) const
{	

	sf::View view(sf::FloatRect{ (sf::Vector2f)viewPosition, (sf::Vector2f)ctx.window.getSize() });
	uint32_t tileSide = windowSize / LE_map.size().x;
	ctx.window.setView(view);
	for (size_t y = 0; y < LE_map.size().y; ++y) for (size_t x = 0; x < LE_map.size().x; ++x)
	{
		sf::Vector2u position(x, y);
		if (LE_map.at(position).sprite != "")
		{
			auto sprite(api.assets.tileSprites.get(LE_map.at(position).sprite).get(0, api.assets.textures));

			sprite.setPosition(sf::Vector2f{
				static_cast<float>(x * tileSide),
				static_cast<float>(y * tileSide)
			});

			auto bounds = sprite.getLocalBounds();
			sprite.setScale(sf::Vector2f{
				static_cast<float>(tileSide) / bounds.size.x,
				static_cast<float>(tileSide) / bounds.size.y
			});

			ctx.window.draw(sprite);
		}
	}
	sf::RectangleShape playerShape;
	playerShape.setSize(playerRect.size * float(tileSide));
	playerShape.setPosition(playerRect.position * float(tileSide));
	playerShape.setFillColor(sf::Color::Blue);
	ctx.window.draw(playerShape);

}

void Scenes::LevelEditor::resizeMap() {
	auto oldMap = LE_map;

	LE_map = px::Grid<Tile>(sf::Vector2u(mapWidth, mapHeight), m_ctx.tiles.at("empty"));

	for (unsigned int y = 0; y < std::min(oldMap.size().y, LE_map.size().y); y++)
	{
		for (unsigned int x = 0; x < std::min(oldMap.size().x, LE_map.size().x); x++)
		{
			LE_map.at({ x, y }) = oldMap.at({ x, y });
		}
	}
}

