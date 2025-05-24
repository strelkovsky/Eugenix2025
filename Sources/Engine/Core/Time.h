#pragma once

#include <chrono>

namespace Eugenix
{
	namespace Time
	{
		using Clock = std::chrono::high_resolution_clock;
		using Duration = std::chrono::duration<float>;
	} // namespace Time
} // namespace Eugenix
