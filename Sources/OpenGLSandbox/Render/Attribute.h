#pragma once

#include <cstdint>

namespace Eugenix
{
	namespace Render
	{
		struct Attribute
		{
			uint32_t index{};
			int32_t  size{};
			uint32_t type{};
			bool normalized;
			uint32_t offset{};
		};
	} // namespace Render
} // namespace Eugenix
