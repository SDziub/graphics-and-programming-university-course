#pragma once

#include <tuple>
#include <optional>

#include <entt/entt.hpp>

namespace px
{
	// Maybe add const getters

	template <typename ... Components>
	class EntityPrefab
	{
	public:

		entt::entity spawn(entt::registry& registry) const
		{
			auto entity = registry.create();

			(([&] {
				auto& opt = std::get<std::optional<Components>>(m_components);
				if (opt) {
					if constexpr (std::is_empty_v<Components>)
						registry.emplace<Components>(entity);
					else
						registry.emplace<Components>(entity, *opt);
				}
			}()), ...);

			return entity;
		}

		template <typename C>
		void set(C&& component)
		{
			std::get<std::optional<C>>(m_components) = std::move(component);
		}

		template <typename C>
		void set(const C& component)
		{
			std::get<std::optional<C>>(m_components) = component;
		}

		template <typename C, typename ... Args>
		void emplace(Args&&... args)
		{
			std::get<std::optional<C>>(m_components) = C(std::forward<Args>(args) ...);
		}

		template <typename C>
		void remove()
		{
			std::get<std::optional<C>>(m_components) = {};
		}

		template <typename C>
		bool has() const
		{
			return std::get<std::optional<C>>(m_components).has_value();
		}

		template <typename C>
		C& get()
		{
			return std::get<std::optional<C>>(m_components).value();
		}

		template <typename C>
		C* tryGet()
		{
			return has<C>() ? &get<C>() : nullptr;
		}

	private:

		std::tuple<std::optional<Components>...> m_components;
	};
}