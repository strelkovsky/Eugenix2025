#pragma once

#include <optional>

#include "VulkanCommon.h"

namespace Eugenix::Render::Vulkan
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class Adapter final
	{
	public:
		bool Select(VkInstance instance, VkSurfaceKHR surface);

		VkPhysicalDevice Handle() const { return _selectedPhysicalDevice; }
		QueueFamilyIndices Indices() const { return _queueIndices; }

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	private:
		VkPhysicalDevice _selectedPhysicalDevice{ VK_NULL_HANDLE };
		VkPhysicalDeviceMemoryProperties _memoryProperties;
		QueueFamilyIndices _queueIndices{};
	};
} // namespace Eugenix::Render::Vulkan