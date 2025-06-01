#pragma once

#include "VulkanInclude.h"

namespace Eugenix::Render::Vulkan
{
	class Swapchain final
	{
	public:
		static constexpr auto MaxFramesInFlight = 3;

		bool Create(const Adapter& adapter, const Surface& surface, const Device& device, GLFWwindow* window);
		void Destroy(VkDevice device);

		VkSwapchainKHR Handle() const { return _swapchain; }
		VkExtent2D Extent() const { return _extent; }
		VkFormat Format() const { return _surfaceFormat.format; }
		const std::vector<VkImage>& Images() const { return _images; }
		const std::vector<VkImageView>& ImageViews() const { return _imageViews; }

	private:
		VkSwapchainKHR _swapchain{ VK_NULL_HANDLE };

		VkExtent2D _extent{};
		VkSurfaceFormatKHR _surfaceFormat{};
		VkPresentModeKHR _presentMode{};

		std::vector<VkImage> _images;
		std::vector<VkImageView> _imageViews;
	};
} // namespace Eugenix::Render::Vulkan
