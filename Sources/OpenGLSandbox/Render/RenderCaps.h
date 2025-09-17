#pragma once

#include <cstdint>

namespace Eugenix::Render
{
	struct Caps final
	{
		float maxAnisotropy{};
		int maxSamples{};
		int maxSamplers{};
	};
} // namespadce Eugenix::Render
