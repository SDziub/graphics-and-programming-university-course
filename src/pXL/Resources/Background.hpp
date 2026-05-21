#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

namespace px
{
	class Background;

	struct BackgroundLayer
	{
		const sf::Texture& texture;
		float speed;
		float movePerSecond;
	};

	class BackgroundData
	{
	public:

		BackgroundData(std::vector<BackgroundLayer>&& textures) :
			m_textures(std::move(textures))
		{}

	private:

		std::vector<BackgroundLayer> m_textures;

		friend Background;
	};

	class Background : public sf::Drawable
	{
	public:

		Background(const BackgroundData& data, const float xPosition, sf::Time elapsed = sf::Time::Zero) : 
			m_data(data),
			m_xPosition(xPosition),
			m_elapsed(elapsed)
		{}

	private:

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			for (const auto& [texture, offset, move] : m_data.m_textures)
			{
				const auto texSize = static_cast<sf::Vector2f>(texture.getSize());
				const float scale = static_cast<float>(target.getSize().y) / texSize.y;
				const float width = texSize.x * scale;
				const float height = texSize.y * scale;

				const float textureOffsetX = ((m_xPosition / scale) * offset) + move * scale * m_elapsed.asSeconds();

				sf::VertexArray quad(sf::PrimitiveType::TriangleStrip, 4);

				quad[0].position = { 0.f, 0.f };
				quad[1].position = { 0.f, height };
				quad[2].position = { width, 0.f };
				quad[3].position = { width, height };

				quad[0].texCoords = { textureOffsetX, 0.f };
				quad[1].texCoords = { textureOffsetX, texSize.y };
				quad[2].texCoords = { textureOffsetX + texSize.x, 0.f };
				quad[3].texCoords = { textureOffsetX + texSize.x, texSize.y };

				sf::RenderStates states;
				states.texture = &texture;

				target.draw(quad, states);
			}
		}

		const BackgroundData& m_data;
		float m_xPosition;
		sf::Time m_elapsed;

		friend sf::RenderTarget;
	};
}