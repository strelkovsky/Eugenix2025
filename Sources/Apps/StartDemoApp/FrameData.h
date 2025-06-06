#pragma once

#include "Render/Vulkan/VulkanInclude.h"

struct FrameData
{
	VkSemaphore imageAvailable;
	VkSemaphore renderFinished;
	VkFence inFlight;
	VkCommandBuffer commandBuffer;
};
