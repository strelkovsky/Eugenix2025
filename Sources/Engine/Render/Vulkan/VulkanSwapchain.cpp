#include <GLFW/glfw3.h>

#include "Core/Log.h"

#include "VulkanAdapter.h"
#include "VulkanDevice.h"
#include "VulkanInitializers.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "VulkanSwapchainUtils.h"

namespace
{
	VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));

			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
				availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}
}

namespace Eugenix::Render::Vulkan
{
	bool Swapchain::Create(const Adapter& adapter, const Surface& surface, const Device& device, GLFWwindow* window)
	{
		SwapchainSupportDetails support = QuerySwapchainSupport(adapter.Handle(), surface.Handle());
		if (support.formats.empty() || support.presentModes.empty())
		{
			LogError("Swapchain support is inadequate (formats or present modes missing).");
			return false;
		}

		_extent =  chooseSwapchainExtent(support.capabilities, window);
		_surfaceFormat =  chooseSwapchainSurfaceFormat(support.formats);
		_presentMode = chooseSwapchainPresentMode(support.presentModes);

		LogInfo("Chosen surface format: format = ", _surfaceFormat.format, ", colorSpace = ", _surfaceFormat.colorSpace);
		LogInfo("Chosen present mode: ", _presentMode);
		LogInfo("Chosen extent: ", _extent.width, "x", _extent.height);

		uint32_t imageCount{ support.capabilities.minImageCount + 1 };
		if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount)
		{
			imageCount = support.capabilities.maxImageCount;
		}

		QueueFamilyIndices indices = adapter.Indices();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		VkSwapchainCreateInfoKHR createInfo = SwapchainInfo(surface.Handle(), imageCount, _surfaceFormat, _extent, 1, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = _presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device.Handle(), &createInfo, nullptr, &_swapchain) != VK_SUCCESS)
		{
			LogError("Failed to create swapchain.");
			return false;
		}

		vkGetSwapchainImagesKHR(device.Handle(), _swapchain, &imageCount, nullptr);
		_images.resize(imageCount);
		vkGetSwapchainImagesKHR(device.Handle(), _swapchain, &imageCount, _images.data());

		_imageViews.resize(_images.size());
		for (size_t i = 0; i < _images.size(); ++i)
		{
			_imageViews[i] = device.CreateImageView(_images[i], _surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		LogSuccess("Swapchain created successfully with ", imageCount, " images.");
		return true;
	}

	void Swapchain::Destroy(VkDevice device)
	{
		if (_swapchain)
		{
			for (VkImageView view : _imageViews)
			{
				vkDestroyImageView(device, view, nullptr);
			}
			_imageViews.clear();

			LogSuccess("Swapchain destroyed.");
			vkDestroySwapchainKHR(device, _swapchain, nullptr);
			_swapchain = VK_NULL_HANDLE;
		}
	}
}