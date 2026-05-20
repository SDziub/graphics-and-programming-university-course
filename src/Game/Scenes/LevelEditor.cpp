#include "LevelEditor.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Game/Map.hpp"
#include <spdlog/spdlog.h>

Scenes::LevelEditor::LevelEditor(px::SceneInitCtx ctx, Context& gctx) :
	Scene(ctx),
	m_ctx(gctx),
	LE_world(),
	LE_registry(LE_world.entities),
	LE_map(LE_world.map)
{
	for (const auto& [tilename, _ ] : m_ctx.tiles) {
		TileName.push_back(tilename);
	}

	for (const auto& [entityName, _] : m_ctx.entities.data()) {
		entities.push_back(entityName);
	}

	if (!m_ctx.maps.empty() && currentMap < m_ctx.maps.size()) {
		mapPath = m_ctx.maps[currentMap];
	}

	player = m_ctx.entities.get("player").spawn(LE_registry);
	auto pp = LE_registry.get<Transform>(player);
	LE_registry.emplace<EntityType>(player, EntityType{ "player" });
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

		ImGui::DragInt("##mapheight", &mapHeight, 0.5f,25,100);
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			resizeMap();
		}
		ImGui::DragInt("##mapWidth", &mapWidth, 0.5f, 25, 100);
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			resizeMap();
		}
		
		
		static bool item_highlight = false;
		int item_highlighted_idx = -1;
		if (ImGui::BeginListBox("Choose tile")) 
		{
			for (int n = 0; n < TileName.size(); n++)
			{
				const bool is_selected = (currentTile == n);
				if (ImGui::Selectable(TileName[n].c_str(), is_selected)) {
					currentTile = n;
					selectState = selected::tile;
				}
				if (item_highlight && ImGui::IsItemHovered())
					item_highlighted_idx = n;

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
					
				}
			}
			
			ImGui::EndListBox();
			
			if (ImGui::BeginListBox("Choose map")) 
			{
				for (int n = 0; n < m_ctx.maps.size(); n++)
				{
					const bool is_selected = (currentMap == n);
					if (ImGui::Selectable(m_ctx.maps[n].c_str(), is_selected)) {
						currentMap = n;
						mapPath = m_ctx.maps[currentMap];
					}
					if (item_highlight && ImGui::IsItemHovered())
						item_highlighted_idx = n;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}

			static char buf5[255] = ""; ImGui::InputText("no blank", buf5, sizeof(buf5), ImGuiInputTextFlags_CharsNoBlank);

			if (ImGui::BeginListBox("Choose entity")) {
				for (int e = 0; e < (int)entities.size(); e++) {
					const bool is_selected = (currentEntity == e);
					if (ImGui::Selectable(entities[e].c_str(), is_selected)) {
						currentEntity = e;
						selectState = selected::entity;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}
			
			if (ImGui::Button("New")) {
				newMapName = buf5;
				mapPathS = "./resources/maps/" + newMapName + ".json";
				saveWorld(mapPathS, LE_world);
				m_ctx.maps.push_back(mapPathS);
			}ImGui::SameLine();

			if (ImGui::Button("Save")) {
				saveWorld(mapPath, LE_world);
			}ImGui::SameLine();

			if (ImGui::Button("Load")) {
				loadWorld(mapPath, LE_tiles, LE_world);
				SPDLOG_INFO("Map size after load: {}x{}", LE_world.map.size().x, LE_world.map.size().y);
				SPDLOG_INFO("Tile [0,0]: {}", LE_world.map.at({ 0,0 }).tileName);
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

	static entt::entity draggedEntity = entt::null;
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		sf::Vector2i tileCoord = mousePosition / tileSize;
		bool inBounds = tileCoord.x >= 0 && tileCoord.y >= 0
			&& static_cast<unsigned>(tileCoord.x) < mapWidth
			&& static_cast<unsigned>(tileCoord.y) < mapHeight;

		if (api.mapping.isPressed(px::InputId::MLeft)) {
			draggedEntity = entt::null;
			LE_registry.view<Transform, Hitbox>().each(
				[&](entt::entity e, const Transform& t, const Hitbox& h) {
					sf::FloatRect rect = h.rect;
					rect.position += t.pos;
					if (rect.contains(worldMousePos))
						draggedEntity = e;
				}
			);
		}

		if (api.mapping.isReleased(px::InputId::MLeft)) {
			if (draggedEntity == entt::null && inBounds && selectState == selected::entity) {
				if (currentEntity >= 0 && currentEntity < (int)entities.size()) {
					SPDLOG_INFO("Spawning {} at {},{}", entities[currentEntity], worldMousePos.x, worldMousePos.y);
					auto e = m_ctx.entities.get(entities[currentEntity]).spawn(LE_registry);
					LE_registry.emplace<EntityType>(e, EntityType{ entities[currentEntity] });
					LE_registry.get<Transform>(e).pos = worldMousePos;
				}
			}
			draggedEntity = entt::null;
		}

		if (api.mapping.isHeld(px::InputId::MLeft)) {
			if (draggedEntity != entt::null) {
				auto& t = LE_registry.get<Transform>(draggedEntity);
				t.pos += sf::Vector2f(api.mapping.getMouseDelta()) / float(tileSize);
			}
			else if (inBounds && selectState == selected::tile) {
				LE_map.at(static_cast<sf::Vector2u>(tileCoord)) = m_ctx.tiles.at(TileName[currentTile]);
			}
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
	uint32_t tileSide = windowSize / LE_world.map.size().x;
	ctx.window.setView(view);
	for (size_t y = 0; y < LE_world.map.size().y; ++y) for (size_t x = 0; x < LE_world.map.size().x; ++x)
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

	auto LEview = LE_registry.view<const px::Animation, const Transform>();

	LEview.each([&](const auto& animation, const auto& transform) {
		sf::Vector2f position = transform.pos * static_cast<float>(tileSide);

		sf::Sprite sprite = animation.getSprite().value();
		sprite.setPosition(position);
		ctx.window.draw(sprite);
		});

}

void Scenes::LevelEditor::resizeMap() {
	auto oldMap = LE_world.map;

	LE_world.map = px::Grid<Tile>(sf::Vector2u(mapWidth, mapHeight), m_ctx.tiles.at("empty"));

	for (unsigned int y = 0; y < std::min(oldMap.size().y, LE_world.map.size().y); y++)
	{
		for (unsigned int x = 0; x < std::min(oldMap.size().x, LE_world.map.size().x); x++)
		{
			LE_world.map.at({ x, y }) = oldMap.at({ x, y });
		}
	}
}

