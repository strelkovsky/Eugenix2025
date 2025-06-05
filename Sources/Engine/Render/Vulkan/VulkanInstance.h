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

				VkInstance Handle() const { return _instance; };

				bool LayerSupported(const char* layerName) const;
				bool ExtensionSupported(const char* extensionName) const;

			private:
				VkInstance _instance{ VK_NULL_HANDLE };
				std::vector<VkLayerProperties> _availableLayers;
				std::vector<VkExtensionProperties> _availableExtensions;

#if EUGENIX_DEBUG
				VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };
#endif
			};
		} // namespace Vulkan
	} // namespace Render
} // namespace Eugenix
