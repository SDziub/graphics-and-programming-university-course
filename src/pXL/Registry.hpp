#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <cassert>
#include <optional>

namespace px
{
	template <typename T>
	class Registry
	{
	public:

		Registry() = default;
		Registry(const Registry&) = delete;
		Registry& operator=(const Registry&) = delete;

		void setError(T&& resource)
		{
			m_error = std::move(resource);
		}

		bool exists(const std::string& name) const { return m_resources.count(name); }

		const T& set(const std::string& name, T&& resource)
		{
			m_resources.insert_or_assign(name, std::move(resource));
			return m_resources.at(name);
		}

		const T* tryGet(const std::string& name) const
		{
			if (exists(name))
			{
				return &m_resources.at(name);
			}

			return m_error ? &m_error.value() : nullptr;;
		}

		const T& get(const std::string& name) const
		{
			const T* out = tryGet(name);
			assert(out);
			return *out;
		}

		const std::unordered_map<std::string, T>& data() const
		{
			return m_resources;
		}

	private:

		std::unordered_map<std::string, T> m_resources;
		std::optional<T> m_error;
	};
}