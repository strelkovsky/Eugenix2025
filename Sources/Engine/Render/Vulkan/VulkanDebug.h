#pragma once

#include <span>

#include <vulkan/vulkan.h>

namespace
{
	inline constexpr const char* ValidationLayerArray[] = 
	{
		"VK_LAYER_KHRONOS_validation"
	};

	inline constexpr const char* DebugExtensionArray[] =
	{
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
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

			inline constexpr std::span<const char* const> DebugExtensions
			{
				DebugExtensionArray
			};
		} // namespace Vulkan
	} // namespace Render
} // namespace Eugenix
