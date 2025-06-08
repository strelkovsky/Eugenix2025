#pragma once

#include "VulkanCommon.h"

namespace Eugenix::Render::Vulkan
{
	struct Buffer final
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
	};
} // Eugenix::Render::Vulkan
