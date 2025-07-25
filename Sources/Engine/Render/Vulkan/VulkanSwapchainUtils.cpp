#include "VulkanSwapchainUtils.h"

namespace Eugenix::Render::Vulkan
{
	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapchainSupportDetails details;

		VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities));

		uint32_t formatCount;
		VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr));
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr));
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}
} // namespace Eugenix::Render::Vulkan
