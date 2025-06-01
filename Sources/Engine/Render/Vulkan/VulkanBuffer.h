#pragma once

#include "VulkanInclude.h"

namespace Eugenix::Render::Vulkan
{
	struct Buffer final
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
	};
} // Eugenix::Render::Vulkan
