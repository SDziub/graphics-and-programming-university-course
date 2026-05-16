#pragma once

#include "pXL/pXL.hpp"

#include "Components.hpp"

using EntityPrefab = px::EntityPrefab<
	Hitbox,
	Transform,
	Controllable,
	Lifetime,
	IsParticle,
	px::Animation,
	ColiderType
>;