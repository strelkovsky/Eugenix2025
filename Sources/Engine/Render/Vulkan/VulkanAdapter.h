#pragma once

#include <optional>

#include "VulkanInclude.h"

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

	private:
		VkPhysicalDevice _selectedPhysicalDevice{ VK_NULL_HANDLE };
		QueueFamilyIndices _queueIndices{};
	};
} // namespace Eugenix::Render::Vulkan


/*
// VulkanAdapter.cpp

#include "VulkanPhysicalDevice.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <set>
#include <optional>

namespace Eugenix::Render::Vulkan
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() const { return graphicsFamily && presentFamily; }
	};

	bool Adapter::Select(VkInstance instance, VkSurfaceKHR surface)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			LogError("No Vulkan-compatible GPUs found.");
			return false;
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		LogInfo("Available Vulkan adapters:");
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);

			std::string type;
			switch (props.deviceType)
			{
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: type = "Discrete GPU"; break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: type = "Integrated GPU"; break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: type = "Virtual GPU"; break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU: type = "CPU"; break;
			default: type = "Other"; break;
			}

			LogInfo("  ", props.deviceName, " (", type, ")");
		}

		for (const auto& device : devices)
		{
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& q : queueFamilies)
			{
				if (q.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					indices.graphicsFamily = i;

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if (presentSupport)
					indices.presentFamily = i;

				if (indices.isComplete()) break;
				i++;
			}

			if (!indices.isComplete())
			{
				LogWarn("Skipping adapter (missing required queue families).\n");
				continue;
			}

			uint32_t extCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(extCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExtensions.data());

			std::set<std::string> required = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			for (const auto& ext : availableExtensions)
				required.erase(ext.extensionName);

			if (!required.empty())
			{
				LogWarn("Skipping adapter (missing required extensions).\n");
				continue;
			}

			_selected = device;
			_queueIndices = indices;
			LogSuccess("Selected adapter: ", vkGetPhysicalDeviceProperties(device).deviceName);
			return true;
		}

		LogError("No suitable adapter found.");
		return false;
	}

	VkPhysicalDevice Adapter::Handle() const { return _selected; }
	QueueFamilyIndices Adapter::Indices() const { return _queueIndices; }
} // namespace Eugenix::Render::Vulkan


*/