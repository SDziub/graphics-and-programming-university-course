#pragma once

namespace px
{
	template <typename T>
	T lerp(T a, T b, float alpha)
	{
		return a + alpha * (b - a);
	}
}