#pragma once

#include "VulkanAdapter.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"

namespace Eugenix
{
	namespace Render
	{
		namespace Vulkan
		{
			struct VulkanAppConfig
			{
				uint32_t apiVersion = VK_API_VERSION_1_4;
				bool enableValidationLayers = true;
			};

			class VulkanApp
			{
			public:
				bool InitVulkan(const VulkanAppConfig& config, GLFWwindow* window)
				{
					if (!_instance.Create(config.apiVersion, config.enableValidationLayers))
						return false;

					if (!_surface.Create(_instance.Handle(), window))
						return false;

					if (!_adapter.Select(_instance.Handle(), _surface.Handle()))
						return false;

					if (!_device.Create(_adapter))
						return false;

					if (!_swapchain.Create(_adapter, _surface, _device, window))
						return false;

					return true;
				}

				void Cleanup()
				{
					_swapchain.Destroy(_device.Handle());
					_device.Destroy();
					_surface.Destroy(_instance.Handle());
					_instance.Destroy();
				}
			protected:
				Instance _instance;
				Surface _surface;
				Adapter _adapter;
				Device _device;
				Swapchain _swapchain;
			};
		}
	} // namespace Render
} // namespace Eugenix
