#include <array>
#include <chrono>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <stb_image.h>

#include <tiny_obj_loader.h>

#include "Render/Vulkan/VulkanApp.h"
#include "Render/Vulkan/VulkanDebug.h"
#include "Render/Vulkan/VulkanInitializers.h"

#include "IO/IO.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT{ 3 };

namespace
{
	inline constexpr const char* DeviceExtensionArray[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
}

inline constexpr std::span<const char* const> DeviceExtensions
{
	DeviceExtensionArray
};

#ifdef _DEBUG
const bool enableValidationLayers{ true };
#else 
const bool enableValidationLayers{ false };
#endif

bool checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : Eugenix::Render::Vulkan::ValidationLayers)
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

std::vector<const char*> getRequiredExtensions()
{
	uint32_t glfwExtensionCount{ 0 };
	const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
	else
	{
		throw std::runtime_error("Failed to get vkDestroyDebugUtilsMessengerEXT function!\n");
	}
}

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDesc{};

		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(Vertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDesc;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDesc{};

		//position
		attributeDesc[0].binding = 0;
		attributeDesc[0].location = 0;
		attributeDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDesc[0].offset = offsetof(Vertex, pos);

		//normal
		attributeDesc[1].binding = 0;
		attributeDesc[1].location = 1;
		attributeDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDesc[1].offset = offsetof(Vertex, normal);

		//color
		attributeDesc[2].binding = 0;
		attributeDesc[2].location = 2;
		attributeDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDesc[2].offset = offsetof(Vertex, texCoord);

		return attributeDesc;
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}
};

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct UniformBufferObject
{
	glm::mat4 view;
	glm::mat4 proj;
};

struct Renderable 
{
	glm::mat4 modelMatrix;
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	uint32_t indexCount;
	VkDescriptorSet descriptorSet;
};

std::vector<Renderable> _renderables;

class Camera {
public:
	glm::vec3 position = { 0.0f, 5.0f, 0.0f };
	glm::vec3 front = { 0.0f, 0.0f, -1.0f };
	glm::vec3 up = { 0.0f, 1.0f, 0.0f };
	glm::vec3 right = { 1.0f, 0.0f, 0.0f };
	glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };

	float yaw = -90.0f;
	float pitch = -90.0f;

	float moveSpeed = 2.5f;
	float mouseSensitivity = 0.1f;

	Camera() { updateVectors(); }

	glm::mat4 getViewMatrix() const {
		return glm::lookAt(position, position + front, up);
	}

	void processKeyboard(char key, float deltaTime) {
		float velocity = moveSpeed * deltaTime;
		if (key == 'W') position += front * velocity;
		if (key == 'S') position -= front * velocity;
		if (key == 'A') position -= right * velocity;
		if (key == 'D') position += right * velocity;
	}

	void processMouse(float xoffset, float yoffset) {
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		yaw += xoffset;
		pitch += yoffset;

		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		updateVectors();
	}

private:
	void updateVectors() {
		glm::vec3 f;
		f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		f.y = sin(glm::radians(pitch));
		f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(f);
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}
};

Camera camera;
float lastX = 0.0f, lastY = 0.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.processMouse(xoffset, yoffset);
}

void processInput(GLFWwindow* window)
{
	auto deltaTime = 0.001f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard('W', deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard('S', deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard('A', deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard('D', deltaTime);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
	{
		glfwSetWindowShouldClose(window, 1);
	}
}

class SampleApp final : public Eugenix::Render::Vulkan::VulkanApp
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* _window{ nullptr };

	VkInstance _instance{ VK_NULL_HANDLE };
	VkDebugUtilsMessengerEXT _debugMessenger{ VK_NULL_HANDLE };

	VkSurfaceKHR _surface{ VK_NULL_HANDLE };

	VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };

	VkDevice _device{ VK_NULL_HANDLE };
	VkQueue _graphicsQueue{ VK_NULL_HANDLE };
	VkQueue _presentQueue{ VK_NULL_HANDLE };

	VkSwapchainKHR _swapchain{ VK_NULL_HANDLE };
	std::vector<VkImage> _swapchainImages;
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;

	std::vector<VkImageView> _swapchainImageViews;

	VkRenderPass _renderPass{ VK_NULL_HANDLE };
	VkPipelineLayout _pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline _graphicsPipeline{ VK_NULL_HANDLE };

	std::vector<VkFramebuffer> _swapchainFramebuffers;
	VkCommandPool _commandPool{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> _commandBuffers;

	std::vector<VkSemaphore> _imageAvailableSemaphores;
	std::vector<VkSemaphore> _renderFinishedSemaphores;
	std::vector<VkFence> _inFlightFences;
	size_t _currentFrame{ 0 };
	bool _resized{ false };

	VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
	VkDescriptorSetLayout _globalDescriptorSetLayout;
	VkDescriptorSet _globalDescriptorSet;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer _vertexBuffer;
	VkDeviceMemory _vertexBufferMemory;
	VkBuffer _indexBuffer;
	VkDeviceMemory _indexBufferMemory;
	
	VkBuffer _uniformBuffer;
	VkDeviceMemory _uniformBufferMemory;

	VkImage _textureImage;
	VkDeviceMemory _textureImageMemory;
	VkImageView _textureImageView;
	VkSampler _textureSampler;

	VkImage _depthImage;
	VkDeviceMemory _depthImageMemory;
	VkImageView _depthImageView;

	double _lastTime;
	int _frameCount{0};

	void initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		_window = glfwCreateWindow(WIDTH, HEIGHT, "Eugenix", nullptr, nullptr);
		glfwSetWindowUserPointer(_window, this);
		glfwSetFramebufferSizeCallback(_window, windowResize);

		glfwSetCursorPosCallback(_window, mouse_callback);
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // скрываем курсор для "free look"
	}

	static void windowResize(GLFWwindow* window, int width, int height)
	{
		auto parent = reinterpret_cast<SampleApp*>(glfwGetWindowUserPointer(window));
		parent->_resized = true;
	}

	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createSwapchainImageViews(); // TODO : merge with createSwapchain	
		createDescriptorSetLayout();
		createCommandPool();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		createDepthResources();
		createRenderPass();
		createFramebuffers();
		createGraphicsPipeline();
		initRenderables();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObject();
	}

	void createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!\n");
		}

		VkApplicationInfo appInfo = Eugenix::Render::Vulkan::ApplicationInfo(
			"Eugenix", VK_MAKE_VERSION(1, 0, 0),
			"Eugenix Engine", VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_4);

		const auto extensions = getRequiredExtensions();
		VkInstanceCreateInfo instanceInfo = Eugenix::Render::Vulkan::InstanceInfo(appInfo, enableValidationLayers, extensions);

		VkResult res = vkCreateInstance(&instanceInfo, nullptr, &_instance);
		if (res != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance!\n");
		}
	}

	void setupDebugMessenger()
	{
		if (enableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = Eugenix::Render::Vulkan::DebugMessengerInfo(
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, 
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, 
				debugCallback);

			if (CreateDebugUtilsMessengerEXT(_instance, &debugMessengerInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to set up debug messenger!\n");
			}
		}
	}

	void pickPhysicalDevice()
	{
		uint32_t deviceCount{ 0 };
		vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find any GPUs with Vulkan support!\n");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				_physicalDevice = device;
				break;
			}
		}

		if (_physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find a suitable GPU!\n");
		}
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount{ 0 };
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported{ checkDeviceExtensionSupport(device) };

		// TODO : print features and other GPU info
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		bool swapchainAdequate{ false };
		if (extensionsSupported)
		{
			SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
			swapchainAdequate = !(swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty());
		}

		return indices.isComplete() && swapchainAdequate && supportedFeatures.samplerAnisotropy;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount{ 0 };
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i{ 0 };
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport{ false };
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			++i;
		}
		return indices;
	}

	void createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

		const float queuePriority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			queueCreateInfos.push_back(Eugenix::Render::Vulkan::QueueInfo(indices.graphicsFamily.value(), 1, queuePriority));
		}

		// NOTE: check in isDeviceSuitable
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = Eugenix::Render::Vulkan::DeviceInfo(queueCreateInfos, deviceFeatures, DeviceExtensions);

		if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device!\n");
		}

		vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
		vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
	}

	SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device)
	{
		SwapchainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

		uint32_t formatCount{ 0 };
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount{ 0 };
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(_window, &width, &height);

			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));

			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
				availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void createSwapchain()
	{
		SwapchainSupportDetails swapchainSupport{ querySwapchainSupport(_physicalDevice) };

		VkExtent2D extent{ chooseSwapchainExtent(swapchainSupport.capabilities) };
		VkSurfaceFormatKHR surfaceFormat{ chooseSwapchainSurfaceFormat(swapchainSupport.formats) };
		VkPresentModeKHR presentMode{ chooseSwapchainPresentMode(swapchainSupport.presentModes) };

		uint32_t imageCount{ swapchainSupport.capabilities.minImageCount + 1 };
		if (swapchainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapchainSupport.capabilities.maxImageCount)
		{
			imageCount = swapchainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = Eugenix::Render::Vulkan::SwapchainInfo(
			_surface, imageCount, surfaceFormat, extent, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

		QueueFamilyIndices indices{ findQueueFamilies(_physicalDevice) };
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swapchain!\n");
		}

		vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
		_swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data());

		_swapchainImageFormat = surfaceFormat.format;
		_swapchainExtent = extent;
	}

	void createSwapchainImageViews()
	{
		_swapchainImageViews.resize(_swapchainImages.size());

		for (size_t i = 0; i < _swapchainImages.size(); ++i)
		{
			_swapchainImageViews[i] = createImageView(_swapchainImages[i], _swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void createFramebuffers()
	{
		_swapchainFramebuffers.resize(_swapchainImageViews.size());

		for (size_t i = 0; i < _swapchainImages.size(); ++i)
		{
			std::array<VkImageView, 2> attachments = { _swapchainImageViews[i], _depthImageView };

			VkFramebufferCreateInfo framebufferInfo = Eugenix::Render::Vulkan::FrameBufferInfo(_renderPass,
				attachments, _swapchainExtent, 1);

			if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer!\n");
			}
		}
	}

	void recreateSwapchain()
	{
		int width{ 0 }, height{ 0 };
		glfwGetFramebufferSize(_window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(_window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(_device);

		cleanupSwapchain();

		createSwapchain();
		createSwapchainImageViews();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		createDepthResources();
		createRenderPass();
		createFramebuffers();
		createGraphicsPipeline();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		
		createCommandBuffers();
	}

	void createRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = _swapchainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::array<VkSubpassDescription, 1> subpasses = { subpass };

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkSubpassDependency, 1> dependencies = { dependency };

		VkRenderPassCreateInfo renderPassInfo = Eugenix::Render::Vulkan::RenderPassInfo(
			attachments, subpasses, dependencies);

		if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass\n");
		}
	}

	void createDescriptorSetLayout()
	{
		// Ubo
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		// Sampler
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = Eugenix::Render::Vulkan::DescriptorSetLayoutInfo(bindings);

		if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_globalDescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!\n");
		}
	}

	void createDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};

		// ubo pool size
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(_swapchainImages.size());

		// sampler pool size
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(_swapchainImages.size());

		VkDescriptorPoolCreateInfo poolInfo = Eugenix::Render::Vulkan::DescriptorPoolInfo(
			poolSizes, static_cast<uint32_t>(_swapchainImages.size()));

		if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool!\n");
		}
	}

	void createDescriptorSets()
	{
		std::array<VkDescriptorSetLayout, 1> setLayouts = { _globalDescriptorSetLayout };

		VkDescriptorSetAllocateInfo allocInfo = Eugenix::Render::Vulkan::DescriptorSetAllocateInfo(_descriptorPool,
			1, setLayouts);

		if (vkAllocateDescriptorSets(_device, &allocInfo, &_globalDescriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!\n");
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = _uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = _textureImageView;
		imageInfo.sampler = _textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0] = Eugenix::Render::Vulkan::WriteDescriptorSet(_globalDescriptorSet, 0, 0, 1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfo);
		descriptorWrites[1] = Eugenix::Render::Vulkan::WriteDescriptorSet(_globalDescriptorSet, 1, 0, 1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageInfo);

		vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void createGraphicsPipeline()
	{
		auto vertShaderCode = Eugenix::IO::FileContent("Shaders/Vulkan/vertex.spv", std::ios::binary);
		auto fragfShaderCode = Eugenix::IO::FileContent("Shaders/Vulkan/fragment.spv", std::ios::binary);

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragfShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = Eugenix::Render::Vulkan::ShaderStageInfo(
			VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule, "main");

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = Eugenix::Render::Vulkan::ShaderStageInfo(
			VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule, "main");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

		std::array<VkVertexInputBindingDescription, 1> bindigsDescs = { Vertex::getBindingDescription() };
		auto attributeDescs = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = Eugenix::Render::Vulkan::VertexInputStateInfo(
			bindigsDescs, attributeDescs);

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = Eugenix::Render::Vulkan::InputAssemplyInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(_swapchainExtent.width);
		viewport.height = static_cast<float>(_swapchainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		std::array<VkViewport, 1> viewports = { viewport };

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = _swapchainExtent;

		std::array<VkRect2D, 1> scissors = { scissor };

		VkPipelineViewportStateCreateInfo viewportState = Eugenix::Render::Vulkan::ViewportStateInfo(viewports, scissors);

		VkPipelineRasterizationStateCreateInfo rasterizer = Eugenix::Render::Vulkan::RasterizationStateInfo(
			VK_FALSE, VK_POLYGON_MODE_FILL, VK_FALSE, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
			VK_FALSE, 0.0f, 0.0f, 0.0f);

		VkPipelineMultisampleStateCreateInfo multisampling = Eugenix::Render::Vulkan::MultisampleStateInfo(
			VK_FALSE, VK_SAMPLE_COUNT_1_BIT, 1.0f, nullptr, VK_FALSE, VK_FALSE);

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineDepthStencilStateCreateInfo depthStencilAttachment = Eugenix::Render::Vulkan::DepthStencilStateInfo(
			VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE);

		std::array<VkPipelineColorBlendAttachmentState, 1> colorAttachments = { colorBlendAttachment };
		VkPipelineColorBlendStateCreateInfo colorBlending = Eugenix::Render::Vulkan::ColorBlendStateInfo(
			VK_FALSE, VK_LOGIC_OP_COPY, colorAttachments, { 0.0f, 0.0f, 0.0f, 0.0f });

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4);

		std::array<VkDescriptorSetLayout, 1> setLayouts = { _globalDescriptorSetLayout };
		std::array<VkPushConstantRange, 1> pushConstantRanges = { pushConstantRange };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Eugenix::Render::Vulkan::PipelineLayoutInfo(setLayouts, 
			pushConstantRanges);

		if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout!\n");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = Eugenix::Render::Vulkan::PipelineInfo(shaderStages,
			vertexInputInfo, inputAssembly, viewportState, rasterizer, multisampling, depthStencilAttachment,
			colorBlending, nullptr, _pipelineLayout, _renderPass, 0, VK_NULL_HANDLE, -1);

		if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!\n");
		}

		vkDestroyShaderModule(_device, vertShaderModule, nullptr);
		vkDestroyShaderModule(_device, fragShaderModule, nullptr);
	}

	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = Eugenix::Render::Vulkan::ShaderModuleInfo(code);

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module!\n");
		}
		return shaderModule;
	}

	void initRenderables()
	{
		loadModel();

		createVertexBuffer();
		createIndexBuffer();

		Renderable obj1;
		obj1.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f));
		obj1.modelMatrix = glm::scale(obj1.modelMatrix, glm::vec3(0.5f));
		obj1.modelMatrix = glm::rotate(obj1.modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		obj1.modelMatrix = glm::rotate(obj1.modelMatrix, glm::radians(-90.0f), glm::vec3(0, 0, 1));
		obj1.vertexBuffer = _vertexBuffer;
		obj1.indexBuffer = _indexBuffer;
		obj1.indexCount = static_cast<uint32_t>(indices.size());
		obj1.descriptorSet = _globalDescriptorSet;

		Renderable obj2;
		obj2.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
		obj2.modelMatrix = glm::scale(obj2.modelMatrix, glm::vec3(0.5f));
		obj2.modelMatrix = glm::rotate(obj2.modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		obj2.modelMatrix = glm::rotate(obj2.modelMatrix, glm::radians(-90.0f), glm::vec3(0, 0, 1));
		obj2.vertexBuffer = _vertexBuffer;
		obj2.indexBuffer = _indexBuffer;
		obj2.indexCount = static_cast<uint32_t>(indices.size());
		obj2.descriptorSet = _globalDescriptorSet;

		_renderables = { obj1, obj2 };
	}

	void loadModel()
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warning, error;

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, "models/viking_room.obj"))
		{
			throw std::runtime_error(warning + error);
		}

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.pos =
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.normal =
				{
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.texCoord =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

	void createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice);

		VkCommandPoolCreateInfo poolInfo = Eugenix::Render::Vulkan::CommandPoolInfo(queueFamilyIndices.graphicsFamily.value(),
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!\n");
		}
	}

	void createCommandBuffers()
	{
		_commandBuffers.resize(_swapchainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = Eugenix::Render::Vulkan::CommandBufferAllocateInfo(_commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(_commandBuffers.size()));

		if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!\n");
		}
	}

	void createSyncObject()
	{
		_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = Eugenix::Render::Vulkan::SemaphoreInfo();
		VkFenceCreateInfo fenceInfo = Eugenix::Render::Vulkan::FenceInfo(VK_FENCE_CREATE_SIGNALED_BIT);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create semaphores!\n");
			}
		}
	}

	VkCommandBuffer beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo = Eugenix::Render::Vulkan::CommandBufferAllocateInfo(_commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = Eugenix::Render::Vulkan::CommandBufferBeginInfo(
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		//VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		return commandBuffer;
	}

	void endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkCommandBuffer commandBuffers[] = { commandBuffer };
		VkSubmitInfo submitInfo = Eugenix::Render::Vulkan::SubmitInfo(commandBuffers);

		vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(_graphicsQueue);

		vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
	}

	void createVertexBuffer()
	{
		VkDeviceSize size = sizeof(Vertex) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(_device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(size));
		vkUnmapMemory(_device, stagingBufferMemory);

		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			_vertexBuffer, _vertexBufferMemory);

		copyBuffer(stagingBuffer, _vertexBuffer, size);
		vkDestroyBuffer(_device, stagingBuffer, nullptr);
		vkFreeMemory(_device, stagingBufferMemory, nullptr);
	}

	void createIndexBuffer()
	{
		VkDeviceSize size = sizeof(uint32_t) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(_device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(size));
		vkUnmapMemory(_device, stagingBufferMemory);

		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			_indexBuffer, _indexBufferMemory);

		copyBuffer(stagingBuffer, _indexBuffer, size);
		vkDestroyBuffer(_device, stagingBuffer, nullptr);
		vkFreeMemory(_device, stagingBufferMemory, nullptr);
	}

	void createUniformBuffers()
	{
		// Camera UBO
		VkDeviceSize size = sizeof(UniformBufferObject);

		createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			_uniformBuffer, _uniformBufferMemory);
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
		VkDeviceMemory& memory)
	{
		VkBufferCreateInfo bufferInfo = Eugenix::Render::Vulkan::BufferCreateInfo(size, usage, VK_SHARING_MODE_EXCLUSIVE);
		if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = Eugenix::Render::Vulkan::MemoryAllocateInfo(memRequirements.size, 
			findMemoryType(memRequirements.memoryTypeBits, properties));

		if (vkAllocateMemory(_device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory!\n");
		}

		vkBindBufferMemory(_device, buffer, memory, 0);
	}

	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type\n");
	}

	void createTextureImage()
	{
		int width, height, channels;
		stbi_uc* pixels = stbi_load("models/viking_room.png", &width, &height, &channels, STBI_rgb_alpha);
		
		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!\n");
		}

		VkDeviceSize imageSize = width * height * 4;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(_device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<uint32_t>(imageSize));
		vkUnmapMemory(_device, stagingBufferMemory);
		stbi_image_free(pixels);

		createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _textureImage, _textureImageMemory);

		transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer, _textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(_device, stagingBuffer, nullptr);
		vkFreeMemory(_device, stagingBufferMemory, nullptr);
	}

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout/*, uint32_t mipLevels*/)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = Eugenix::Render::Vulkan::ImageMemoryBarrier(oldLayout, newLayout,
			VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!\n");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
	}

	void createTextureImageView()
	{
		_textureImageView = createImageView(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = aspect;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(_device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views!\n");
		}

		return imageView;
	}

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(_device, image, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory!\n");
		}

		vkBindImageMemory(_device, image, imageMemory, 0);
	}

	void createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 4;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(_device, &samplerInfo, nullptr, &_textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler!\n");
		}
	}

	void createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();

		createImage(_swapchainExtent.width, _swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
		_depthImageView = createImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	VkFormat findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT }, // список форматов в порядке предпочтения
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}


	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!\n");
	}

	void createSurface()
	{
		if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface!\n");
		}
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(_window))
		{
			glfwPollEvents();
			processInput(_window);
			drawFrame();
			updateFPS(_window);
		}
		vkDeviceWaitIdle(_device);
	}

	void updateFPS(GLFWwindow* window)
	{
		double currentTime = glfwGetTime();
		double delta = currentTime - _lastTime;

		_frameCount++;
		if (delta >= 1.0) // if last update was more than one second ago
		{
			double fps = double(_frameCount) / delta;

			std::stringstream ss;
			ss << "Eugenix. FPS: " << fps;

			glfwSetWindowTitle(window, ss.str().c_str());

			_frameCount = 0;

			_lastTime = currentTime;
		}
	}

	void drawFrame()
	{
		vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex{};
		vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

		updatePerFrameData();
		updateUniformBuffer(imageIndex);

		vkResetCommandBuffer(_commandBuffers[_currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];
		VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		
		vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);
		if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit draw command buffer!\n");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { _swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		VkResult result{ vkQueuePresentKHR(_presentQueue, &presentInfo) };
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _resized)
		{
			_resized = false;
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Rendering failed!\n");
		}

		_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!\n");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _renderPass;
		renderPassInfo.framebuffer = _swapchainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _swapchainExtent;
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.5f, 0.0f, 0.25f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

		//VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		for (const auto& renderable : _renderables)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &renderable.vertexBuffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, renderable.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &renderable.modelMatrix);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_globalDescriptorSet, 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(renderable.indexCount), 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!\n");
		}
	}


	void updatePerFrameData()
	{
		static auto lastFrame = (float)glfwGetTime();

		// TODO : move deltaTime to VulkanApp
		float currentFrame = (float)glfwGetTime();
		auto deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		_renderables[0].modelMatrix = glm::rotate(_renderables[0].modelMatrix, deltaTime, glm::vec3(0, 0, 1));
		_renderables[1].modelMatrix = glm::rotate(_renderables[1].modelMatrix, deltaTime, glm::vec3(0, 0, 1));
	}

	void updateUniformBuffer(uint32_t currentImage)
	{
		UniformBufferObject ubo{};
		ubo.view = camera.getViewMatrix();
		ubo.proj = glm::perspective(glm::radians(45.0f), float(_swapchainExtent.width) / float(_swapchainExtent.height), 0.1f, 100.0f);
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(_device, _uniformBufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(_device, _uniformBufferMemory);
	}

	void cleanup()
	{
		cleanupSwapchain();

		vkDestroySampler(_device, _textureSampler, nullptr);

		vkDestroyImageView(_device, _textureImageView, nullptr);
		vkDestroyImage(_device, _textureImage, nullptr);
		vkFreeMemory(_device, _textureImageMemory, nullptr);

		vkDestroyImageView(_device, _depthImageView, nullptr);
		vkDestroyImage(_device, _depthImage, nullptr);
		vkFreeMemory(_device, _depthImageMemory, nullptr);

		vkDestroyBuffer(_device, _vertexBuffer, nullptr);
		vkFreeMemory(_device, _vertexBufferMemory, nullptr);

		vkDestroyBuffer(_device, _indexBuffer, nullptr);
		vkFreeMemory(_device, _indexBufferMemory, nullptr);

		vkDestroyDescriptorSetLayout(_device, _globalDescriptorSetLayout, nullptr);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(_device, _inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(_device, _commandPool, nullptr);

		vkDestroyDevice(_device, nullptr);

		if (enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);

		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	void cleanupSwapchain()
	{
		vkDestroyBuffer(_device, _uniformBuffer, nullptr);
		vkFreeMemory(_device, _uniformBufferMemory, nullptr);

		vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

		for (auto framebuffer : _swapchainFramebuffers)
		{
			vkDestroyFramebuffer(_device, framebuffer, nullptr);
		}

		vkFreeCommandBuffers(_device, _commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());

		vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
		vkDestroyRenderPass(_device, _renderPass, nullptr);

		for (VkImageView imageView : _swapchainImageViews)
		{
			vkDestroyImageView(_device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	}
};

int main()
{
	SampleApp app;
	app.run();
	return 0;
}