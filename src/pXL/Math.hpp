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

	struct SweptAABBResult
	{
		sf::Vector2f normal;
		float value{};
		bool accured{};
	};

	SweptAABBResult sweptAABB(sf::FloatRect moving, sf::FloatRect stationary, sf::Vector2f deltaDistance)
	{
		sf::Vector2f inverseEntry, inverseExit;

		if (deltaDistance.x > 0.0f)
		{
			inverseEntry.x = stationary.position.x - (moving.position.x + moving.size.x);
			inverseExit.x = (stationary.position.x + stationary.size.x) - moving.position.x;
		}
		else
		{
			inverseEntry.x = (stationary.position.x + stationary.size.x) - moving.position.x;
			inverseExit.x = stationary.position.x - (moving.position.x + moving.size.x);
		}

		if (deltaDistance.y > 0.0f)
		{
			inverseEntry.y = stationary.position.y - (moving.position.y + moving.size.y);
			inverseExit.y = (stationary.position.y + stationary.size.y) - moving.position.y;
		}
		else
		{
			inverseEntry.y = (stationary.position.y + stationary.size.y) - moving.position.y;
			inverseExit.y = stationary.position.y - (moving.position.y + moving.size.y);
		}

		sf::Vector2f entry, exit;

		if (deltaDistance.x == 0.0f)
		{
			entry.x = -std::numeric_limits<float>::infinity();
			exit.x = std::numeric_limits<float>::infinity();
		}
		else
		{
			entry.x = inverseEntry.x / deltaDistance.x;
			exit.x = inverseExit.x / deltaDistance.x;
		}

		if (deltaDistance.y == 0.0f)
		{
			entry.y = -std::numeric_limits<float>::infinity();
			exit.y = std::numeric_limits<float>::infinity();
		}
		else
		{
			entry.y = inverseEntry.y / deltaDistance.y;
			exit.y = inverseExit.y / deltaDistance.y;
		}

		float entryTime = std::max(entry.x, entry.y);
		float exitTime = std::min(exit.x, exit.y);

		SweptAABBResult result;
		result.accured = !(entryTime > exitTime
			|| (entry.x < 0.0f && entry.y < 0.0f)
			|| entry.x > 1.0f
			|| entry.y > 1.0f);

		if (!result.accured)
		{
			return result;
		}

		if (entry.x > entry.y)
		{
			if (inverseEntry.x < 0.0f)
			{
				result.normal.x = 1.0f;
			}
			else
			{
				result.normal.x = -1.0f;
			}
		}
		else
		{
			if (inverseEntry.y < 0.0f)
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
}