#pragma once

#include <span>

namespace Eugenix::Core
{
	struct Data
	{
		const void* ptr{};
		size_t      size{};
	};

	template <class T, std::size_t N>
	inline Data MakeData(std::span<const T, N> s) 
	{
		return { s.data(), s.size_bytes() };
	}

	template <typename type>
	static Data MakeData(std::span<const type> span)
	{
		return{ span.data(), span.size_bytes() };
	}
} // namespace Eugenix::Render
