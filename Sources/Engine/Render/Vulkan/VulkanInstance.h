#pragma once

#include "VulkanInclude.h"
#include "VulkanUtils.h"

namespace Eugenix
{
	namespace Render
	{
		namespace Vulkan
		{
			class Instance final
			{
			public:
				Instance() = default;

				bool Create(uint32_t apiVersion, bool enableValidationLayers);
				void Destroy();

				VkInstance Native() const { return _instance; };

			private:
				VkInstance _instance{ VK_NULL_HANDLE };
				VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };
			};
		} // namespace Vulkan
	} // namespace Render
} // namespace Eugenix
