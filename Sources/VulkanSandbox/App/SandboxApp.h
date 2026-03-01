#pragma once

#include "Render/Vulkan/EugenixVulkan.h"

namespace Eugenix
{
	struct AppConfig
	{

	};

	class SandboxApp
	{
	public:
		SandboxApp(const AppConfig& config = {})
		{

		}
	protected:
		VkInstance _instance{ VK_NULL_HANDLE };
		VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
		VkDevice _device{ VK_NULL_HANDLE };
	};
} // namespace Eugenix
