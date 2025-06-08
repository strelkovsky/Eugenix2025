#pragma once

#include "Render/Vulkan/VulkanCommon.h"

struct FrameData
{
	VkSemaphore imageAvailable;
	VkSemaphore renderFinished;
	VkFence inFlight;
	VkCommandBuffer commandBuffer;
};
