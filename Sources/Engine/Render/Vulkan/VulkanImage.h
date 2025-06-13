#pragma once

#include "VulkanCommon.h"

namespace Eugenix::Render::Vulkan
{
	struct Image
	{
		VkImage image{ VK_NULL_HANDLE };
		VkImageView view{ VK_NULL_HANDLE };

		VkDeviceMemory memory{ VK_NULL_HANDLE };
	};
} // Eugenix::Render::Vulkan
