#include <set>
#include <vector>

#include "Core/Log.h"

#include "VulkanAdapter.h"
#include "VulkanSwapchainUtils.h"

namespace Eugenix::Render::Vulkan
{
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

			SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(device, surface);
			if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
			{
				LogWarn("Skipping adapter (inadequate swapchain support).\n");
				continue;
			}

			_selectedPhysicalDevice = device;
			_queueIndices = indices;

			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);
			LogSuccess("Selected adapter: ", props.deviceName);
			return true;
		}

		LogError("No suitable adapter found.");
		return false;
	}
} // namespace Eugenix::Render::Vulkan