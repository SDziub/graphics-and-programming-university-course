#pragma once

#include "pXL/pXL.hpp"

namespace Scenes
{
	class Settings : public px::Scene
	{
	public:

		Settings(px::SceneInitCtx& ctx, px::Mapping& mapping) :
			px::Scene(ctx),
			m_mapping(mapping)
		{}

		void update(px::UpdateCtx& ctx) override
		{

		}

		void draw(px::DrawCtx& ctx) const override
		{

		}

	private:

		px::Mapping& m_mapping;
	};
}