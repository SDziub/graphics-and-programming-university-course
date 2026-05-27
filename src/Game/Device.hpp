#pragma once

#include "pXl/pXL.hpp"

class Device
{
public:

	virtual void update(px::UpdateCtx& ctx) = 0;
};