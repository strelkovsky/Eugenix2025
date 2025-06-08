#pragma once

#include <vulkan/vulkan.h>

#include "Core/Log.h"

#define EUGENIX_VULKAN_ALLOCATOR nullptr

#define VERIFYVULKANRESULT(VkFunction) { const VkResult scopedResult = VkFunction; if (scopedResult != VK_SUCCESS) { Eugenix::LogError("VKResult=", scopedResult, ", Function=", #VkFunction, ", File=", __FILE__, ", Line=", __LINE__); }}