#pragma once

#include <span>
#include <vector>

namespace Eugenix
{
	template<typename T>
	void AppendSpan(std::vector<T>& dst, std::span<const T> src)
	{
		dst.insert(dst.end(), src.begin(), src.end());
	}
} // namespace Eugenix
