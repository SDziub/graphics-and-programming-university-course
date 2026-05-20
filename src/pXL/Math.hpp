#pragma once

#include <limits>
#include <algorithm>

#include <SFML/Graphics/Rect.hpp>

namespace px
{
	template <typename T>
	T lerp(T a, T b, float alpha)
	{
		return a + alpha * (b - a);
	}

	template <typename T>
	bool colideAABB(sf::Rect<T> a, sf::Rect<T> b)
	{
		return a.position.x <= b.position.x + b.size.x
			&& a.position.y <= b.position.y + b.size.y
			&& a.position.x + a.size.x >= b.position.x
			&& a.position.y + a.size.y >= b.position.y;
	}

	struct ColisionResult
	{
		sf::Vector2f point, normal;
		float time;
		bool hit{};
	};

	inline ColisionResult rectangleRaycast(const sf::Vector2f origin, const sf::Vector2f deltaDistance, const sf::FloatRect rectangle)
	{
		sf::Vector2f timeNear = (rectangle.position - origin);
		timeNear.x /= deltaDistance.x;
		timeNear.y /= deltaDistance.y;
		sf::Vector2f timeFar = rectangle.position + rectangle.size - origin;
		timeFar.x /= deltaDistance.x;
		timeFar.y /= deltaDistance.y;

		ColisionResult result;

		if (std::isnan(timeNear.x) || std::isnan(timeNear.y) || std::isnan(timeFar.x) || std::isnan(timeFar.y))
		{
			return result;
		}

		if (timeNear.x > timeFar.x)
		{
			std::swap(timeNear.x, timeFar.x);
		}
		if (timeNear.y > timeFar.y)
		{
			std::swap(timeNear.y, timeFar.y);
		}

		if (timeNear.x > timeFar.x || timeNear.y > timeFar.y)
		{
			return result;
		}

		result.time = std::max(timeNear.x, timeNear.y);
		float remainingTime = std::min(timeFar.x, timeFar.y);

		if (remainingTime < 0.0f || result.time > 1.0f || result.time < 0.0f)
		{
			return result;
		}

		result.hit = true;
		result.point = origin + result.time * deltaDistance;

		if (timeNear.x > timeNear.y)
		{
			if (deltaDistance.x < 0.0f)
			{
				result.normal.x = 1.0f;
			}
			else
			{
				result.normal.x = -1.0f;
			}
		}
		else if (timeNear.x < timeNear.y)
		{
			if (deltaDistance.y < 0.0f)
			{
				result.normal.y = 1.0f;
			}
			else
			{
				result.normal.y = -1.0f;
			}
		}

		return result;
	}

	inline ColisionResult sweptAABB(const sf::FloatRect dynamic, const sf::FloatRect stationary, const sf::Vector2f deltaDistance)
	{
		ColisionResult result;

		if (deltaDistance.x == 0.0f && deltaDistance.y == 0.0f)
		{
			return result;
		}

		sf::FloatRect expandedStationary{
			stationary.position - dynamic.size / 2.0f,
			stationary.size + dynamic.size
		};

		result = rectangleRaycast(dynamic.position + dynamic.size / 2.0f, deltaDistance, expandedStationary);

		return result;
	}
}