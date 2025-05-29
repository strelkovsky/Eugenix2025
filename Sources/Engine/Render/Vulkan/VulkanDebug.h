#pragma once

#include <span>

#include <vulkan/vulkan.h>

namespace
{
	inline constexpr const char* ValidationLayerArray[] = 
	{
		"VK_LAYER_KHRONOS_validation"
	};
}

namespace Eugenix
{
	namespace Render
	{
		namespace Vulkan
		{
			inline constexpr std::span<const char* const> ValidationLayers
			{
				ValidationLayerArray
			};
		} // namespace Vulkan
	} // namespace Render
} // namespace Eugenix
