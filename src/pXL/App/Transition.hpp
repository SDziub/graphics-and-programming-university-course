#pragma once

#include <functional>

#include <SFML/Graphics.hpp>

namespace px
{
	class Engine;

	class Transition : public sf::Drawable
	{
	public:

		void start(std::function<void()> callback);
		bool isMidpoint() const;

	private:

		void update(float dt);
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

		enum class State : uint8_t { None, BeforeMidpoint, Midpoint, AfterMidpoint };

		std::function<void()> m_callback{};
		float m_progress{};
		State m_state{};

		friend Engine;
	};

	inline void Transition::start(std::function<void()> callback)
	{
		m_callback = callback;
		m_state = State::BeforeMidpoint;
	}

	inline bool Transition::isMidpoint() const
	{
		return m_state == State::Midpoint;
	}

	inline void Transition::update(float dt)
	{
		if (m_state == State::BeforeMidpoint)
		{
			m_progress += 7.5f * dt;
			if (m_progress > 1.0f)
			{
				m_progress = 1.0f;
				m_state = State::Midpoint;
			}
		}
		else if (m_state == State::Midpoint)
		{
			if (m_callback)
			{
				m_callback();
			}

			m_state = State::AfterMidpoint;
		}
		else if (m_state == State::AfterMidpoint)
		{
			m_progress -= 7.5f * dt;
			if (m_progress < 0.0f)
			{
				m_progress = 0.0f;
				m_state = State::None;
			}
		}
	}

	inline void Transition::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		sf::RectangleShape blackRect(static_cast<sf::Vector2f>(target.getSize()));
		blackRect.setFillColor(sf::Color(0, 0, 0, 255 * m_progress));
		target.draw(blackRect);
	}
}