#include <iostream>

#include <GLFW/glfw3.h>

#include "Core/Containers.h"
#include "Core/Log.h"
#include "Core/Platform.h"

#include "VulkanInitializers.h"
#include "VulkanInstance.h"

namespace
{
#ifdef EUGENIX_DEBUG
	static constexpr std::array validationLayers = 
	{
		"VK_LAYER_KHRONOS_validation"
	};

	static constexpr std::array debugExtensions =
	{
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};
#endif

	std::vector<VkLayerProperties> getAvailableInstanceLayers()
	{
		uint32_t instanceLayerCount = 0;
		VERIFYVULKANRESULT(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
		std::vector<VkLayerProperties> availableLayers(instanceLayerCount);
		VERIFYVULKANRESULT(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableLayers.data()));

		return availableLayers;
	}

	std::vector<VkExtensionProperties> getAvailableInstanceExtensions()
	{
		uint32_t count = 0;
		VERIFYVULKANRESULT(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
		std::vector<VkExtensionProperties> extensions(count);
		VERIFYVULKANRESULT(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));

		return extensions;
	}

	std::vector<const char*> getRequiredLayers(bool enableValidationLayers)
	{
		std::vector<const char*> layers;

#ifdef EUGENIX_DEBUG
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

#ifdef EUGENIX_DEBUG
		Eugenix::AppendSpan(extensions, std::span<const char* const>(debugExtensions));
#endif // EUGENIX_DEBUG

		return extensions;
	}

#ifdef EUGENIX_DEBUG
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
		case Eugenix::LogSeverity::Error:   Eugenix::LogError("Vulkan: ", data->pMessage); break;
		case Eugenix::LogSeverity::Warning: Eugenix::LogWarn("Vulkan: ", data->pMessage); break;
		case Eugenix::LogSeverity::Info:    Eugenix::LogInfo("Vulkan: ", data->pMessage); break;
		case Eugenix::LogSeverity::Verbose: Eugenix::LogVerbose("Vulkan: ", data->pMessage); break;
		}
		return VK_FALSE;
	}
#endif
}

namespace Eugenix::Render::Vulkan
{
	bool Instance::Create(uint32_t apiVersion, bool enableValidationLayers)
	{
		_availableLayers = getAvailableInstanceLayers();
		LogInfo("Available instance layers:");
		for (const auto& prop : _availableLayers)
			LogInfo("  ", prop.layerName);

		_availableExtensions = getAvailableInstanceExtensions();
		Eugenix::LogInfo("Available instance extensions:");
		for (const auto& prop : _availableExtensions)
		{
			Eugenix::LogInfo("  ", prop.extensionName);
		}

		const auto requiredLayers = getRequiredLayers(enableValidationLayers);
		Eugenix::LogInfo("Required layers:");
		for (const auto& layer : requiredLayers)
		{
			Eugenix::LogInfo("  ", layer);
			if (!LayerSupported(layer))
			{
				LogError("Unsupported Layer - ", layer);
			}
		}

		const auto requiredExtensions = getRequiredExtensions();
		Eugenix::LogInfo("Required extensions:");
		for (const auto& ext : requiredExtensions)
		{
			Eugenix::LogInfo("  ", ext);
			if (!ExtensionSupported(ext))
			{
				LogError("Unsupported Extension - ", ext);
			}
		}

		VkApplicationInfo appInfo = ApplicationInfo("Eugenix", VK_MAKE_VERSION(1, 0, 0),
			"Eugenix Engine", VK_MAKE_VERSION(1, 0, 0), apiVersion);
				
		VkInstanceCreateInfo instanceInfo = InstanceInfo(appInfo, requiredLayers, requiredExtensions);
		auto res = vkCreateInstance(&instanceInfo, nullptr, &_instance);
		if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
			throw std::runtime_error("Cannot find a compatible Vulkan driver (ICD).");
		else if (res != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan instance!\n");

		LogSuccess("Vulkan instance created successfully.");

#ifdef EUGENIX_DEBUG
		if (enableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = DebugMessengerInfo(
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				DebugCallback);

			VERIFYVULKANRESULT(CreateDebugUtilsMessengerEXT(_instance, &debugMessengerInfo, nullptr, &_debugMessenger));

			LogSuccess("Debug messenger initialized.");
		}
#endif

		return true;
	}

	void Instance::Destroy()
	{
#ifdef EUGENIX_DEBUG
		if (_debugMessenger)
		{
			LogSuccess("Debug messenger destroyed.");
			DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
		}
#endif

		if (_instance)
		{
			LogSuccess("Vulkan instance destroyed.");
			vkDestroyInstance(_instance, nullptr);
		}
	}

	bool Instance::LayerSupported(const char* layerName) const
	{
		for (const auto& i : _availableLayers)
		{
			if (strcmp(i.layerName, layerName) == 0)
				return true;
		}
		return false;
	}

	bool Instance::ExtensionSupported(const char* extensionName) const
	{
		for (const auto& prop : _availableExtensions)
		{
			if (strcmp(prop.extensionName, extensionName) == 0)
				return true;
		}
		return false;
	}
} // namespace Eugenix::Render::Vulkan
