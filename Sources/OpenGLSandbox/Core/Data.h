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
	static Data MakeData(const type* ptr)
	{
		assert(ptr != nullptr);
		return { ptr, sizeof(type) };
	}

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

	template <typename type>
	static Data MakeData(const std::vector<type>& vector)
	{
		return{ vector.data(), vector.size() * sizeof(type) };
	}

	template<class T, class Alloc> 
	static Data MakeData(const std::vector<T, Alloc>*) = delete;
	template<class T, class Alloc> 
	static Data MakeData(std::vector<T, Alloc>*) = delete;
	template<class T, size_t N>
	static Data MakeData(const std::span<T, N>*) = delete;
	template<class T>          
	static Data MakeData(const std::span<T>*) = delete;
} // namespace Eugenix::Core
