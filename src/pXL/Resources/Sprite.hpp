#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

namespace px
{
	struct AnimationFrame
	{
		sf::IntRect rectangle;
		sf::Time time;
	};

	enum class AnimationClipType
	{
		Normal,
		Looped,
		Sticky
	};

	struct AnimationClip
	{
		const sf::Texture& texture;
		std::vector<AnimationFrame> frames;
		AnimationClipType type{};
		sf::Vector2f origin{};
	};

	struct AnimationClipMap
	{
		std::unordered_map<std::string, AnimationClip> map;
		std::string fallback;
	};

	class Animation {
	public:

		explicit Animation(const AnimationClipMap& clips)
			: m_clips(&clips)
		{
			play(clips.fallback);
		}

		void setMirrored(bool mirrored)
		{
			m_mirrored = mirrored;
		}

		bool isMirrored() const
		{
			return m_mirrored;
		}

		void play(const std::string& name)
		{
			if (m_currentClip == name && !m_finished)
			{
				return;
			}

			auto it = m_clips->map.find(name);
			if (it == m_clips->map.end())
			{
				return;
			}

			m_currentClip = name;
			m_frameIndex = 0;
			m_elapsed = sf::Time::Zero;
			m_finished = false;
		}

		void update(sf::Time dt)
		{
			if (m_finished)
			{
				return;
			}
			
			auto it = m_clips->map.find(m_currentClip);
			if (it == m_clips->map.end())
			{
				return;
			}

			const auto& clip = it->second;
			if (clip.frames.empty())
			{
				return;
			}

			m_elapsed += dt;

			while (m_elapsed >= clip.frames[m_frameIndex].time)
			{
				m_elapsed -= clip.frames[m_frameIndex].time;
				++m_frameIndex;

				if (m_frameIndex >= clip.frames.size()) {
					switch (clip.type) {
					case AnimationClipType::Looped:
						m_frameIndex = 0;
						break;

					case AnimationClipType::Normal:
						m_frameIndex = clip.frames.size() - 1;
						m_finished = true;
						m_elapsed = sf::Time::Zero;
						play(m_clips->fallback);
						return;

					case AnimationClipType::Sticky:
						m_frameIndex = clip.frames.size() - 1;
						m_elapsed = sf::Time::Zero;
						break;
					}
					break;
				}
			}
		}

		std::optional<sf::Sprite> getSprite() const
		{
			const AnimationClip& clip = m_clips->map.at(m_currentClip);
			if (clip.frames.empty())
			{
				return {};
			}

			sf::IntRect clipRect = clip.frames[m_frameIndex].rectangle;

			if (m_mirrored)
			{
				int32_t width = clipRect.size.x;
				clipRect.size.x = -width;
				clipRect.position.x += width;
			}

			sf::Sprite sprite(clip.texture, clipRect);
			sprite.setOrigin(clip.origin);

			return sprite;
		}

	private:

		const AnimationClipMap* m_clips;
		std::string m_currentClip;
		size_t m_frameIndex{};
		sf::Time m_elapsed{};
		bool m_mirrored{};
		bool m_finished{};
	};
}