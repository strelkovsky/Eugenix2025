#include "Core/Log.h"

#include "VulkanSurface.h"

bool Eugenix::Render::Vulkan::Surface::Create(VkInstance instance, GLFWwindow* window)
{
	if (glfwCreateWindowSurface(instance, window, EUGENIX_VULKAN_ALLOCATOR, &_surface) != VK_SUCCESS)
	{
		LogError("Failed to create window surface.");
		return false;
	}
	LogSuccess("Window surface created.");
    return true;
}

void Eugenix::Render::Vulkan::Surface::Destroy(VkInstance instance)
{
	if (_surface)
	{
		LogSuccess("Window surface destroyed.");
		vkDestroySurfaceKHR(instance, _surface, EUGENIX_VULKAN_ALLOCATOR);
		_surface = VK_NULL_HANDLE;
	}
}
