#pragma once

#include <span>

#include "VulkanDebug.h"
#include "VulkanInclude.h"

namespace
{
	inline VkWriteDescriptorSet WriteDescriptorSetBase(VkDescriptorSet descriptorSet, uint32_t binding,
		uint32_t arrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType)
	{
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.dstArrayElement = arrayElement;
		writeDescriptorSet.descriptorCount = descriptorCount;
		writeDescriptorSet.descriptorType = descriptorType;

		return writeDescriptorSet;
	}
}

namespace Eugenix
{
	namespace Render
	{
		namespace Vulkan
		{
			inline VkApplicationInfo ApplicationInfo(const char* appName, uint32_t appVersion, const char* engineName, uint32_t engineVersion, uint32_t apiVersion)
			{
				VkApplicationInfo appInfo{};
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pEngineName = engineName;
				appInfo.engineVersion = engineVersion;
				appInfo.pApplicationName = appName;
				appInfo.applicationVersion = appVersion;
				appInfo.apiVersion = apiVersion;
				return appInfo;
			}

			inline VkInstanceCreateInfo InstanceInfo(const VkApplicationInfo& appInfo, bool enableValidationLayers, std::span<const char* const> extensions)
			{
				VkInstanceCreateInfo instanceInfo{};
				instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				instanceInfo.pApplicationInfo = &appInfo;

				if (enableValidationLayers)
				{
					instanceInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
					instanceInfo.ppEnabledLayerNames = ValidationLayers.data();
				}
				else
				{
					instanceInfo.enabledLayerCount = 0;
				}

				instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
				instanceInfo.ppEnabledExtensionNames = extensions.data();

				return instanceInfo;
			}

			inline VkDebugUtilsMessengerCreateInfoEXT DebugMessengerInfo(VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType, PFN_vkDebugUtilsMessengerCallbackEXT callback)
			{
				VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
				debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				debugMessengerInfo.messageSeverity = messageSeverity;
				debugMessengerInfo.messageType = messageType;
				debugMessengerInfo.pfnUserCallback = callback;

				return debugMessengerInfo;
			}

			inline VkDeviceQueueCreateInfo QueueInfo(uint32_t familyIndex, uint32_t count, float priority)
			{
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = familyIndex;
				queueInfo.queueCount = count;
				queueInfo.pQueuePriorities = &priority;

				return queueInfo;
			}

			inline VkDeviceCreateInfo DeviceInfo(std::span<const VkDeviceQueueCreateInfo> queueInfos,
				VkPhysicalDeviceFeatures deviceFeatures, std::span<const char* const> extensions)
			{
				VkDeviceCreateInfo deviceInfo{};
				deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
				deviceInfo.pQueueCreateInfos = queueInfos.data();
				deviceInfo.pEnabledFeatures = &deviceFeatures;
				deviceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
				deviceInfo.ppEnabledExtensionNames = extensions.data();
				return deviceInfo;
			}

			inline VkSwapchainCreateInfoKHR SwapchainInfo(VkSurfaceKHR surface, uint32_t imageCount, 
				VkSurfaceFormatKHR surfaceFormat, VkExtent2D extent, uint32_t imageArrayLayers, 
				VkImageUsageFlags imageUsage)
			{
				VkSwapchainCreateInfoKHR swapchainInfo{};
				swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchainInfo.surface = surface;
				swapchainInfo.minImageCount = imageCount;
				swapchainInfo.imageFormat = surfaceFormat.format;
				swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
				swapchainInfo.imageExtent = extent;
				swapchainInfo.imageArrayLayers = imageArrayLayers;
				swapchainInfo.imageUsage = imageUsage;
				return swapchainInfo;
			}

			inline VkFramebufferCreateInfo FrameBufferInfo(VkRenderPass renderPass, std::span<const VkImageView> attachments,
				VkExtent2D extent, uint32_t layers)
			{
				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = extent.width;
				framebufferInfo.height = extent.height;
				framebufferInfo.layers = layers;

				return framebufferInfo;
			}

			inline VkRenderPassCreateInfo RenderPassInfo(std::span<const VkAttachmentDescription> attachments,
				std::span<const VkSubpassDescription> subpasses, std::span<const VkSubpassDependency> dependencies)
			{
				VkRenderPassCreateInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderPassInfo.pAttachments = attachments.data();
				renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
				renderPassInfo.pSubpasses = subpasses.data();
				renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
				renderPassInfo.pDependencies = dependencies.data();

				return renderPassInfo;
			}

			inline VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutInfo(std::span<const VkDescriptorSetLayoutBinding> bindings)
			{
				VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
				descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
				descriptorSetLayoutInfo.pBindings = bindings.data();

				return descriptorSetLayoutInfo;
			}

			inline VkDescriptorPoolCreateInfo DescriptorPoolInfo(std::span<const VkDescriptorPoolSize> sizes, uint32_t maxSets)
			{
				VkDescriptorPoolCreateInfo descriptorPoolInfo{};
				descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
				descriptorPoolInfo.pPoolSizes = sizes.data();
				descriptorPoolInfo.maxSets = maxSets;

				return descriptorPoolInfo;
			}

			inline VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool,
				uint32_t descriptorSetCount, std::span<const VkDescriptorSetLayout> setLayouts)
			{
				VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
				descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAllocateInfo.descriptorPool = descriptorPool;
				descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
				descriptorSetAllocateInfo.pSetLayouts = setLayouts.data();

				return descriptorSetAllocateInfo;
			}

			inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding,
				uint32_t arrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, 
				VkDescriptorBufferInfo bufferInfo)
			{
				VkWriteDescriptorSet writeDescriptorSet = WriteDescriptorSetBase(descriptorSet,
					binding, arrayElement, descriptorCount, descriptorType);
				writeDescriptorSet.pBufferInfo = &bufferInfo;

				return writeDescriptorSet;
			}

			inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding,
				uint32_t arrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType,
				 VkDescriptorImageInfo imageInfo)
			{
				VkWriteDescriptorSet writeDescriptorSet = WriteDescriptorSetBase(descriptorSet,
					binding, arrayElement, descriptorCount, descriptorType);
				writeDescriptorSet.pImageInfo = &imageInfo;

				return writeDescriptorSet;
			}

			inline VkPipelineShaderStageCreateInfo ShaderStageInfo(VkShaderStageFlagBits stage,
				VkShaderModule module, const char* entry)
			{
				VkPipelineShaderStageCreateInfo shaderStageInfo{};
				shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageInfo.stage = stage;
				shaderStageInfo.module = module;
				shaderStageInfo.pName = entry;

				return shaderStageInfo;
			}

			inline VkPipelineVertexInputStateCreateInfo VertexInputStateInfo(
				std::span<const VkVertexInputBindingDescription> bindingDescriptions,
				std::span<const VkVertexInputAttributeDescription> attributeDescriptions)
			{
				VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
				vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				vertexInputStateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
				vertexInputStateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
				vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
				vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

				return vertexInputStateInfo;
			}

			inline VkPipelineInputAssemblyStateCreateInfo InputAssemplyInfo(VkPrimitiveTopology topology,
				VkBool32 primitiveRestartEnable)
			{
				VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
				inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				inputAssemblyInfo.topology = topology;
				inputAssemblyInfo.primitiveRestartEnable = primitiveRestartEnable;

				return inputAssemblyInfo;
			}

			inline VkPipelineViewportStateCreateInfo ViewportStateInfo(std::span<const VkViewport> viewports,
				std::span<const VkRect2D> scissors)
			{
				VkPipelineViewportStateCreateInfo viewportStateInfo{};
				viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewportStateInfo.viewportCount = static_cast<uint32_t>(viewports.size());
				viewportStateInfo.pViewports = viewports.data();
				viewportStateInfo.scissorCount = static_cast<uint32_t>(scissors.size());
				viewportStateInfo.pScissors = scissors.data();

				return viewportStateInfo;
			}
		} // namespace vulkan
	} // namespace Render
} // namespace Eugenix
