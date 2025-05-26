#pragma once

#include <vector>

namespace Eugenix
{
	namespace Render
	{
		struct Data
		{
			const void* ptr{};
			size_t      size{};
		};

		template <typename type> 
		static Data MakeData(const std::vector<type>& buffer)
		{
			return { buffer.data(), buffer.size() * sizeof(type) };
		}

		template <typename type>
		static Data MakeData(const type* ptr)
		{
			return { ptr, sizeof(type) };
		}
	} // namespace Render
} // namespace Eugenix
