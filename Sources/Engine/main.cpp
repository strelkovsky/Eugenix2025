#include <iostream>

#include <vulkan/vulkan.h>

int main()
{
	std::cout << "Hello, Vulkan!" << std::endl;

	VkInstance instance{ VK_NULL_HANDLE };

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Eugenix";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "Eugenix Engine";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 4, 0);

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;

	VkResult res = vkCreateInstance(&instanceInfo, nullptr, &instance);
	if (res != VK_SUCCESS)
	{
		std::cout << "Failed to create VkInstance!" << std::endl;
	}

	return 0;
}