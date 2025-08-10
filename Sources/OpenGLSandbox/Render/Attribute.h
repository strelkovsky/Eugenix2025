#pragma once

#include <cstdint>

#include "Types.h"

namespace Eugenix::Render
{
	struct Attribute
	{
		uint32_t index{};
		int32_t  size{};
		DataType type{};
		bool normalized;
		uint32_t offset{};
	};
} // namespace Eugenix::Render
