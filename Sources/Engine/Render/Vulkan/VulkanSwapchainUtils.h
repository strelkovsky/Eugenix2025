#pragma once

#include <vector>

#include "VulkanCommon.h"

namespace Eugenix::Render::Vulkan
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
} // namespace Eugenix::Render::Vulkan
