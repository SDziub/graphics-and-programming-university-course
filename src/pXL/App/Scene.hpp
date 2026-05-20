#pragma once

#include <optional>
#include <any>

namespace sf
{
	class RenderTarget;
	class Window;
}

namespace px
{
	class InputRaw;
	class Assets;
	class SceneCommands;
	class SceneConfig;
	class Transition;
	class Mapping;

	struct EngineApi
	{
		SceneCommands& comms;
		const Assets& assets;
		const Mapping& mapping;
		const float& unit;
	};

	class Scene;
	class Client;

	class SceneInitCtx
	{
	public:

		SceneInitCtx(SceneConfig& properties, Transition& transition, EngineApi api) :
			properties(properties),
			transition(transition),
			api(api)
		{}

		SceneConfig& properties;
		Transition& transition;

	private:

		EngineApi api;

		friend Scene;
		friend Client;
	};

	struct UpdateCtx
	{
		const sf::Window& window;
		const sf::Time dt;
		Transition& transition;
	};

	struct DrawCtx
	{
		sf::RenderTarget& window;
		const Assets& assets;
		float alpha{};
	};

	class SceneStack;

	class Scene
	{
	public:

		Scene(const SceneInitCtx& ctx) : api(ctx.api) {}
		virtual ~Scene() = default;

		virtual void onEnter(std::any&& payload) {}
		virtual void update(UpdateCtx& ctx) {}
		virtual void fixedUpdate(UpdateCtx& ctx) {}
		virtual void draw(DrawCtx& ctx) const = 0;

	protected:

		EngineApi api;

	private:

		friend SceneStack;
	};
}