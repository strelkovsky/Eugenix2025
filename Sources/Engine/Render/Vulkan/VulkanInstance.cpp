#include <iostream>

#include <GLFW/glfw3.h>

#include "Core/Containers.h"
#include "Core/Log.h"
#include "Core/Platform.h"

#include "VulkanInitializers.h"
#include "VulkanInstance.h"

namespace
{
	static constexpr std::array validationLayers = 
	{
		"VK_LAYER_KHRONOS_validation"
	};

	static constexpr std::array debugExtensions =
	{
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	std::vector<VkLayerProperties> getAvailableInstanceLayers()
	{
		uint32_t instanceLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableLayers.data());

		return availableLayers;
	}

	std::vector<VkExtensionProperties> getAvailableInstanceExtensions()
	{
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> extensions(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

		return extensions;
	}

	std::vector<const char*> getRequiredLayers(bool enableValidationLayers)
	{
		std::vector<const char*> layers;

#if EUGENIX_DEBUG
		Eugenix::AppendSpan(layers, std::span<const char* const>(validationLayers));
#endif // EUGENIX_DEBUG

		return layers;
	}

	std::vector<const char*> getRequiredExtensions()
	{
		// TODO : VK_KHR_SURFACE_EXTENSION_NAME etc

		uint32_t glfwExtensionCount{ 0 };
		const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#if EUGENIX_DEBUG
		Eugenix::AppendSpan(extensions, std::span<const char* const>(debugExtensions));
#endif // EUGENIX_DEBUG

		return extensions;
	}

	bool layerSupported(const char* layer)
	{
		uint32_t instanceLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableLayers.data());

		for (const auto& i : availableLayers)
		{
			if (strcmp(i.layerName, layer) == 0)
				return true;
		}
		return false;
	}

	bool layersSupported(std::span<const char* const> desiredLayers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (auto layerName : desiredLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	bool extensionSupported(const char* extension)
	{
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> extensions(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

		for (const auto& prop : extensions)
		{
			if (strcmp(prop.extensionName, extension) == 0)
				return true;
		}
		return false;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* info, 
		const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* messenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		return func ? func(instance, info, allocator, messenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* allocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			return func(instance, messenger, allocator);
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
	{
		Eugenix::LogSeverity logLevel = Eugenix::LogSeverity::Verbose;
		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			logLevel = Eugenix::LogSeverity::Error;
		else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			logLevel = Eugenix::LogSeverity::Warning;
		else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			logLevel = Eugenix::LogSeverity::Info;

		switch (logLevel)
		{
		case Eugenix::LogSeverity::Error:   Eugenix::LogError(data->pMessage); break;
		case Eugenix::LogSeverity::Warning: Eugenix::LogWarn(data->pMessage); break;
		case Eugenix::LogSeverity::Info:    Eugenix::LogInfo(data->pMessage); break;
		case Eugenix::LogSeverity::Verbose: Eugenix::LogVerbose(data->pMessage); break;
		}
		return VK_FALSE;
	}
}

namespace Eugenix::Render::Vulkan
{
	bool Instance::Create(uint32_t apiVersion, bool enableValidationLayers)
	{
		std::vector<VkLayerProperties> availableLayers = getAvailableInstanceLayers();
		LogInfo("Available instance layers:");
		for (const auto& prop : availableLayers)
			LogInfo("  ", prop.layerName);

		std::vector<VkExtensionProperties> availableExtensions = getAvailableInstanceExtensions();
		Eugenix::LogInfo("Available instance extensions:");
		for (const auto& prop : availableExtensions)
		{
			Eugenix::LogInfo("  ", prop.extensionName);
		}

		const auto requiredLayers = getRequiredLayers(enableValidationLayers);
		Eugenix::LogInfo("Required layers:");
		for (const auto& layer : requiredLayers)
		{
			Eugenix::LogInfo("  ", layer);
			if (!layerSupported(layer))
			{
				LogError("Unsupported Layer - ", layer);
			}
		}

		const auto requiredExtensions = getRequiredExtensions();
		Eugenix::LogInfo("Required extensions:");
		for (const auto& ext : requiredExtensions)
		{
			Eugenix::LogInfo("  ", ext);
			if (!extensionSupported(ext))
			{
				LogError("Unsupported Extension - ", ext);
			}
		}

		VkApplicationInfo appInfo = ApplicationInfo("Eugenix", VK_MAKE_VERSION(1, 0, 0),
			"Eugenix Engine", VK_MAKE_VERSION(1, 0, 0), apiVersion);
				
		VkInstanceCreateInfo instanceInfo = InstanceInfo(appInfo, requiredLayers, requiredExtensions);
		if (vkCreateInstance(&instanceInfo, nullptr, &_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan instance!\n");
		}

		if (enableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = DebugMessengerInfo(
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				DebugCallback);

			if (CreateDebugUtilsMessengerEXT(_instance, &debugMessengerInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to set up debug messenger!\n");
			}
		}

		return true;
	}

	void Instance::Destroy()
	{
		if (_debugMessenger)
			DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);

		if (_instance)
			vkDestroyInstance(_instance, nullptr);
	}
} // namespace Eugenix::Render::Vulkan
