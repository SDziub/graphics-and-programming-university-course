#include "LevelEditor.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Game/Map.hpp"
#include <spdlog/spdlog.h>


Scenes::LevelEditor::LevelEditor(px::SceneInitCtx ctx, Context& gctx) :
	Scene(ctx),
	m_ctx(gctx),
	LE_map(sf::Vector2u(mapWidth, mapHeight), m_ctx.tiles["empty"]) 
{	
	for (const auto& [tilename, _ ] : m_ctx.tiles) {
		TileName.push_back(tilename);
	}
	
	
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

		ImGui::DragInt("##mapheight", &mapHeight, 0.5f,25,100);
		if (ImGui::IsItemDeactivatedAfterEdit) {
			resizeMap();
		}
		ImGui::DragInt("##mapWidth", &mapWidth, 0.5f, 25, 100);
		if (ImGui::IsItemDeactivatedAfterEdit) {
			resizeMap();
		}
		
		
		static bool item_highlight = false;
		int item_highlighted_idx = -1;
		if (ImGui::BeginListBox("Choose tile"))
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
			
			if (ImGui::BeginListBox("Choose map"))
			{
				for (int n = 0; n < maps.size(); n++)
				{
					const bool is_selected = (currentMap == n);
					if (ImGui::Selectable(maps[n].c_str(), is_selected))
						currentMap = n;
						mapName = maps[currentMap];
					if (item_highlight && ImGui::IsItemHovered())
						item_highlighted_idx = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}
			//ImGui::DragInt("##mCurrentMap", &currentMap, 1.f, 0, 2);
			static char buf5[32] = ""; ImGui::InputText("no blank", buf5, sizeof(buf5), ImGuiInputTextFlags_CharsNoBlank);

			//if (ImGui::Button("New")) {
			//	std::ofstream newMap()
			//}
			if (ImGui::Button("Save")) {
				saveMap(mapName, LE_map);
			}

			if (ImGui::Button("Load")) {
				LE_map = loadMap(mapName, LE_tiles);
				SPDLOG_INFO("Map size after load: {}x{}", LE_map.size().x, LE_map.size().y);
				SPDLOG_INFO("Tile [0,0]: {}", LE_map.at({ 0,0 }).tileName);
			}
		}
		
		ImGui::End();
		
	}

	sf::Rect<int> rect{
		sf::Vector2i(0,0), sf::Vector2i((windowSize / mapHeight),(windowSize / mapHeight))
	};

	sf::Vector2i mousePosition = api.mapping.getMousePosition();
	mousePosition += viewPosition;

	if (!ImGui::GetIO().WantCaptureMouse)// to sprawia ze jak klikasz po ui to nie stawiaja sie tile pod spodem
	{
		// sprobuj to zrobic bez tego loopa na bazie pozycji myszki + offsetu widoku oraz wielkosci tilea
		for (unsigned int y = 0; y < mapHeight; y++) {
			for (unsigned int x = 0; x < mapWidth; x++) {
				rect.position = sf::Vector2i(x * (windowSize / mapHeight), y * (windowSize / mapHeight));
				if (api.mapping.isHeld(px::InputId::MLeft) && rect.contains(mousePosition)) {
					sf::Vector2u MP = sf::Vector2u(mousePosition / (windowSize / mapHeight));
					LE_map.at(MP) = m_ctx.tiles.at(TileName[currentTile]);
					//LE_map.set(MP, sceneApi.tiles.handle("solid_block")); //change it to currentTile later.

				}
			}
		}
	}

	sf::Vector2i currMousePos = api.mapping.getMousePosition();
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

