#pragma once

#include <vector>

#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include <Components.hpp>

class Device
{
public:

	std::vector<entt::entity> out;

	Device(entt::registry& registry) :
		m_registry(registry)
	{}

	virtual void compute(sf::Time fixedDt) = 0;

protected:

	void setOutput(bool output)
	{
		for (auto entity : out)
		{
			if (auto* toggle = m_registry.try_get<Toggleable>(entity))
			{
				toggle->active = toggle->negated ? !output : output;
			}
		}
	}

	entt::registry& m_registry;
};

namespace Devices
{
	class And : public Device
	{
	public:

		std::vector<entt::entity> in;

		And(entt::registry& registry) :
			Device(registry)
		{}

		void compute(sf::Time fixedDt) override
		{
			bool on = true;

			for (auto entity : in)
			{
				if (Toggleable* toggle = m_registry.try_get<Toggleable>(entity))
				{
					if (!(toggle->active && !toggle->negated) || (!toggle->active && toggle->negated))
					{
						on = false;
						break;
					}
				}
			}

			setOutput(on);
		}
	};

	class Or : public Device
	{
	public:

		std::vector<entt::entity> in;

		Or(entt::registry& registry) :
			Device(registry)
		{}

		void compute(sf::Time fixedDt) override
		{
			bool on = false;

			for (auto entity : in)
			{
				if (Toggleable* toggle = m_registry.try_get<Toggleable>(entity))
				{
					if (toggle->active && !toggle->negated) || (!toggle->active && toggle->negated)
					{
						on = true;
						break;
					}
				}
			}

			setOutput(on);
		}
	};

	class Timed : public Device
	{
	public:

		sf::Time onTime, offTime, accumulated;
		std::vector<entt::entity> out;

		Timed(entt::registry& registry) :
			Device(registry)
		{}

		void compute(sf::Time fixedDt) override
		{
			sf::Time cycle = onTime + offTime;
			sf::Time thisCycle = accumulated % cycle;

			accumulated += fixedDt;

			bool on = thisCycle <= onTime;

			setOutput(on);
		}
	};
}