#pragma once

#include "VulkanInstance.h"

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
				bool InitVulkan(const VulkanAppConfig& config)
				{
					if (_instance.Create(config.apiVersion, config.enableValidationLayers))
					{
						return false;
					}

					return true;
				}

				void Cleanup()
				{
					_instance.Destroy();
				}
			protected:
				Instance _instance;
			};
		}
	} // namespace Render
} // namespace Eugenix
