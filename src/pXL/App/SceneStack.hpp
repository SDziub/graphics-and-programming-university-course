#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <cassert>
#include <string>
#include <any>
#include <utility>

#include "Scene.hpp"

namespace px
{
	class Client;

	class SceneCommands
	{
	public:

		virtual void push(const std::string& id) = 0;
		virtual void push(const std::string& id, std::any&& payload) = 0;
		virtual void pop() = 0;
		virtual void pop(std::any&& payload) = 0;
		virtual void replace(const std::string& id) = 0;
		virtual void replace(const std::string& id, std::any&& payload) = 0;
		virtual void popUntil(const std::string& id) = 0;
		virtual void popUntil(const std::string& id, std::any&& payload) = 0;
		virtual bool isRequestPending() const = 0;
	};

	class SceneConfig
	{
	public:

		virtual void setTransparency(bool transparent) = 0;
	};

	class SceneStack final : public SceneCommands, public SceneConfig
	{
	public:

		using SceneFactory = std::function<std::unique_ptr<Scene>()>;

		void push(const std::string& id) override;
		void push(const std::string& id, std::any&& payload) override;
		void pop() override;
		void pop(std::any&& payload) override;
		void replace(const std::string& id) override;
		void replace(const std::string& id, std::any&& payload) override;
		void popUntil(const std::string& id) override;
		void popUntil(const std::string& id, std::any&& payload) override;
		bool isRequestPending() const override;

		void registerScene(const std::string& id, SceneFactory factoryFunction);
		bool empty() const;

	private:

		void setTransparency(bool transparent) override;

		enum class SceneAction : uint8_t { Push, Replace, Pop, PopUntil };

		struct SceneRequest
		{
			SceneAction action;
			std::optional<std::string> requested;
			std::any payload;
		};

		struct SceneInstance
		{
			SceneInstance(const std::string& id) : id(id) {}

			std::string id;
			std::unique_ptr<Scene> ptr{};
			bool isTransparent{};
		};

		void pushScene(const std::string& id);
		void popScene();

		void flush();
		void update(UpdateCtx& ctx);
		void fixedUpdate(UpdateCtx& ctx);
		void draw(DrawCtx& ctx) const;
		void setOnChangeCallback(std::function<void()> callback);

		std::unordered_map<std::string, SceneFactory> m_factories;
		std::function<void()> m_onChangeCallback;
		std::vector<SceneInstance> m_scenes;
		std::optional<SceneRequest> m_request;
		
		friend Client;
	};

	inline void SceneStack::push(const std::string& id)
	{
		m_request = { SceneAction::Push, id, {} };
	}

	inline void SceneStack::push(const std::string& id, std::any&& payload)
	{
		m_request = { SceneAction::Push, id, std::move(payload) };
	}

	inline void SceneStack::pop()
	{
		m_request = { SceneAction::Pop, {}, {}};
	}

	inline void SceneStack::pop(std::any&& payload)
	{
		m_request = { SceneAction::Pop, {}, std::move(payload) };
	}

	inline void SceneStack::replace(const std::string& id)
	{
		m_request = { SceneAction::Replace, id, {} };
	}

	inline void SceneStack::replace(const std::string& id, std::any&& payload)
	{
		m_request = { SceneAction::Replace, id, std::move(payload) };
	}

	inline void SceneStack::popUntil(const std::string& id)
	{
		m_request = { SceneAction::PopUntil, id, {} };
	}

	inline void SceneStack::popUntil(const std::string& id, std::any&& payload)
	{
		m_request = { SceneAction::PopUntil, id, std::move(payload) };
	}

	inline bool SceneStack::isRequestPending() const
	{
		return m_request.has_value();
	}

	inline void SceneStack::registerScene(const std::string& id, SceneFactory factoryFunction)
	{
		m_factories.insert_or_assign(id, std::move(factoryFunction));
	}

	inline bool SceneStack::empty() const
	{
		return m_scenes.empty();
	}

	inline void SceneStack::setTransparency(bool transparent)
	{
		assert(!m_scenes.empty() && "Can't set a transparency on a top scene if there is no top scene");
		m_scenes.back().isTransparent = transparent;
	}

	inline void SceneStack::pushScene(const std::string& id)
	{
		assert(m_factories.count(id) && "Can't push() an unregistered scene onto a scene stack");
		m_scenes.emplace_back(id);
		m_scenes.back().ptr = m_factories.at(id)();
	}

	inline void SceneStack::popScene()
	{
		assert(!m_scenes.empty() && "Can't pop() a scene from an empty scene stack");
		m_scenes.pop_back();
	}

	inline void SceneStack::flush()
	{
		if (!m_request)
		{
			return;
		}

		SceneRequest request = m_request.value();
		m_request = {};
		const std::string* requested = request.requested ? &request.requested.value() : nullptr;

		if (request.action == SceneAction::Pop)
		{
			popScene();
		}
		else if (request.action == SceneAction::PopUntil)
		{
			assert(std::any_of(m_scenes.begin(), m_scenes.end(), [&](const SceneInstance& scene) { return scene.id == *requested; }),
				"Can't popUntil() to a scene that's not on the stack");

			while (m_scenes.back().id != *requested)
			{
				popScene();
			}
		}
		else
		{
			if (request.action == SceneAction::Replace)
			{
				popScene();
			}
			pushScene(*requested);
		}

		assert(!m_scenes.empty() && "Can't run onEnter() if the scene stack is empty");
		m_scenes.back().ptr->onEnter(std::move(request.payload));

		if (m_onChangeCallback)
		{
			m_onChangeCallback();
		}
	}

	inline void SceneStack::update(UpdateCtx& ctx)
	{
		assert(!m_scenes.empty() && "Can't run update() a scene if the scene stack is empty");

		m_scenes.back().ptr->update(ctx);
	}

	inline void SceneStack::fixedUpdate(UpdateCtx& ctx)
	{
		assert(!m_scenes.empty() && "Can't run fixedUpdate() a scene if the scene stack is empty");

		m_scenes.back().ptr->fixedUpdate(ctx);
	}

	inline void SceneStack::draw(DrawCtx& ctx) const
	{
		assert(!m_scenes.empty() && "Can't run draw() a scene if a scene stack is empty");

		const size_t count = m_scenes.size();
		size_t firstRenderable = count - 1;
		while (firstRenderable > 0 && m_scenes[firstRenderable].isTransparent)
		{
			--firstRenderable;
		}

		DrawCtx dummyCtx = ctx;
		dummyCtx.alpha = 1.0f;

		for (size_t i = firstRenderable; i < count; ++i)
		{
			if (i == count - 1)
			{
				m_scenes[i].ptr->draw(ctx);
			}
			else
			{
				m_scenes[i].ptr->draw(dummyCtx);
			}
			ctx.window.setView(ctx.window.getDefaultView());
		}
	}

	inline void SceneStack::setOnChangeCallback(std::function<void()> callback)
	{
		m_onChangeCallback = callback;
	}
}