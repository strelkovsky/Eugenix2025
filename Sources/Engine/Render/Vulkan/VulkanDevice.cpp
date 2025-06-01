#include <set>
#include <vector>

#include "Core/Log.h"

#include "VulkanDevice.h"
#include "VulkanInitializers.h"

namespace
{
	static constexpr std::array deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
}

namespace Eugenix::Render::Vulkan
{
	bool Device::Create(const Adapter& adapter)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		const float queuePriority = 1.0f;

		std::set<uint32_t> uniqueQueueFamilies = 
		{ 
			adapter.Indices().graphicsFamily.value(), adapter.Indices().presentFamily.value()
		};

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			queueCreateInfos.push_back(QueueInfo(queueFamily, 1, &queuePriority));
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = DeviceInfo(queueCreateInfos, deviceFeatures, deviceExtensions);

		if (vkCreateDevice(adapter.Handle(), &createInfo, nullptr, &_device) != VK_SUCCESS)
		{
			LogError("Failed to create logical device.");
			return false;
		}

		LogSuccess("Vulkan logical device created successfully.");

		vkGetDeviceQueue(_device, adapter.Indices().graphicsFamily.value(), 0, &_graphicsQueue);
		vkGetDeviceQueue(_device, adapter.Indices().presentFamily.value(), 0, &_presentQueue);

		return true;
	}

	void Device::Destroy()
	{
		if (_device)
		{
			LogSuccess("Logical device destroyed.");
			vkDestroyDevice(_device, nullptr);
			_device = VK_NULL_HANDLE;
			_graphicsQueue = VK_NULL_HANDLE;
			_presentQueue = VK_NULL_HANDLE;
		}
	}
} // namespace Eugenix::Render::Vulkan