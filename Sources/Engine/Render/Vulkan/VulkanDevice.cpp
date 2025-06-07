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
	bool Device::Create(Adapter& adapter)
	{
		_adapter = &adapter;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		const float queuePriority = 1.0f;

		std::set<uint32_t> uniqueQueueFamilies = 
		{ 
			_adapter->Indices().graphicsFamily.value(), _adapter->Indices().presentFamily.value()
		};

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			queueCreateInfos.push_back(QueueInfo(queueFamily, 1, &queuePriority));
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = DeviceInfo(queueCreateInfos, deviceFeatures, deviceExtensions);

		if (vkCreateDevice(_adapter->Handle(), &createInfo, nullptr, &_device) != VK_SUCCESS)
		{
			LogError("Failed to create logical device.");
			return false;
		}

		LogSuccess("Vulkan logical device created successfully.");

		vkGetDeviceQueue(_device, _adapter->Indices().graphicsFamily.value(), 0, &_graphicsQueue);
		vkGetDeviceQueue(_device, _adapter->Indices().presentFamily.value(), 0, &_presentQueue);

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

	VkImageView Device::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect) const
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = aspect;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(_device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views!\n");
		}

		return imageView;
	}

	Buffer Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const
	{
		Buffer buffer{};

		VkBufferCreateInfo bufferInfo = BufferCreateInfo(size, usage);
		if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device, buffer.buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = MemoryAllocateInfo(memRequirements.size, _adapter->FindMemoryType(memRequirements.memoryTypeBits, properties));
		if (vkAllocateMemory(_device, &allocInfo, nullptr, &buffer.memory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory!\n");
		}

		vkBindBufferMemory(_device, buffer.buffer, buffer.memory, 0);

		return buffer;
	}
} // namespace Eugenix::Render::Vulkan