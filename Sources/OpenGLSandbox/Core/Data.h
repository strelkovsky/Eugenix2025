#pragma once

#include <span>

namespace Eugenix::Core
{
	struct Data
	{
		const void* ptr{};
		size_t      size{};
	};

	template <typename type>
	static Data MakeData(std::span<const type> span)
	{
		return{ span.data(), span.size_bytes() };
	}

	template <typename type>
	static Data MakeData(const type* ptr)
	{
		return { ptr, sizeof(type) };
	}
} // namespace Eugenix::Render
