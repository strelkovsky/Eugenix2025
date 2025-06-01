#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanInclude.h"

namespace Eugenix::Render::Vulkan
{
	class Surface final
	{
	public:
		bool Create(VkInstance instance, GLFWwindow* window);
		void Destroy(VkInstance instance);

		VkSurfaceKHR Handle() const { return _surface; };

	private:
		VkSurfaceKHR _surface{ VK_NULL_HANDLE };
	};
} // namespace Eugenix::Render::Vulkan