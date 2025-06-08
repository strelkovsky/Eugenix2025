#pragma once

#include "VulkanAdapter.h"
#include "VulkanBuffer.h"
#include "VulkanCommon.h"

namespace Eugenix::Render::Vulkan
{
	class Device
	{
	public:
		bool Create(Adapter& adapter);
		void Destroy();

		VkDevice Handle() const { return _device; }

		VkQueue GraphicsQueue() const { return _graphicsQueue; }
		VkQueue PresentQueue() const { return _presentQueue; }

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect) const;
		Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;

	private:
		Adapter* _adapter{ nullptr };

		VkDevice _device{ VK_NULL_HANDLE };
		VkQueue _graphicsQueue{ VK_NULL_HANDLE };
		VkQueue _presentQueue{ VK_NULL_HANDLE };
	};
} // namespace Eugenix::Render::Vulkan
