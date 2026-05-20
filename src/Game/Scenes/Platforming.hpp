#pragma once

#include <entt/entt.hpp>

#include "pXL/pXL.hpp"

#include "Game/Context.hpp"
#include "Game/Tile.hpp"
#include "Game/Map.hpp"
#include "Game/Components.hpp"

namespace Scenes
{
	class Platforming : public px::Scene
	{
	public:

		Platforming(px::SceneInitCtx& ctx, Context& gctx);

		void update(px::UpdateCtx& ctx) override;
		void fixedUpdate(px::UpdateCtx& ctx) override;
		void draw(px::DrawCtx& ctx) const override;

	private:

		void playerControlSystem(px::UpdateCtx& api);
		void movementAndColisionSystem(px::UpdateCtx& api);

		Context& m_ctx;

		entt::registry m_registry;

		sf::Time m_elapsed;

		Map m_map;

		sf::Vector2f m_cameraPosition, m_oldCameraPosition;

		std::optional<sf::Time> m_jumpBuffer;
		sf::Time m_floor;

		int32_t m_dir{ 1 };
	};
}