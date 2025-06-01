#pragma once

#include <array>
#include <span>
#include <vector>

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

			inline VkInstanceCreateInfo InstanceInfo(const VkApplicationInfo& appInfo, std::span<const char* const> layers, std::span<const char* const> extensions)
			{
				VkInstanceCreateInfo instanceInfo{};
				instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				instanceInfo.pApplicationInfo = &appInfo;

				instanceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
				instanceInfo.ppEnabledLayerNames = layers.data();

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

			inline VkDeviceQueueCreateInfo QueueInfo(uint32_t familyIndex, uint32_t count, const float* priority)
			{
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = familyIndex;
				queueInfo.queueCount = count;
				queueInfo.pQueuePriorities = priority;

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

			inline VkPipelineRasterizationStateCreateInfo RasterizationStateInfo(VkBool32 depthClampEnable, 
				VkPolygonMode polygonMode, VkBool32 rasterizerDiscardEnable, float lineWidth,
				VkCullModeFlags cullMode, VkFrontFace frontFace, VkBool32 depthBiasEnable,
				float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
			{
				VkPipelineRasterizationStateCreateInfo rasterizationStateInfo{};
				rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				rasterizationStateInfo.depthClampEnable = depthClampEnable;
				rasterizationStateInfo.polygonMode = polygonMode;
				rasterizationStateInfo.rasterizerDiscardEnable = rasterizerDiscardEnable;
				rasterizationStateInfo.lineWidth = lineWidth;
				rasterizationStateInfo.cullMode = cullMode;
				rasterizationStateInfo.frontFace = frontFace;
				rasterizationStateInfo.depthBiasEnable = depthBiasEnable;
				rasterizationStateInfo.depthBiasConstantFactor = depthBiasConstantFactor;
				rasterizationStateInfo.depthBiasClamp = depthBiasClamp;
				rasterizationStateInfo.depthBiasSlopeFactor = depthBiasSlopeFactor;

				return rasterizationStateInfo;
			}

			inline VkPipelineMultisampleStateCreateInfo MultisampleStateInfo(bool sampleShadingEnable,
				VkSampleCountFlagBits rasterizationSamples, float minSampleShading, const VkSampleMask* pSampleMask,
				VkBool32 alphaToCoverageEnable, VkBool32 alphaToOneEnable)
			{
				VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
				multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				multisampleStateInfo.sampleShadingEnable = sampleShadingEnable;
				multisampleStateInfo.rasterizationSamples = rasterizationSamples;
				multisampleStateInfo.minSampleShading = minSampleShading;
				multisampleStateInfo.pSampleMask = pSampleMask;
				multisampleStateInfo.alphaToCoverageEnable = alphaToCoverageEnable;
				multisampleStateInfo.alphaToOneEnable = alphaToOneEnable;

				return multisampleStateInfo;
			}

			inline VkPipelineDepthStencilStateCreateInfo DepthStencilStateInfo(VkBool32 depthTestEnable,
				VkBool32 depthWriteEnable, VkCompareOp depthCompareOp, VkBool32 depthBoundsTestEnable,
				VkBool32 stencilTestEnable)
			{
				VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
				depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				depthStencilStateInfo.depthTestEnable = depthTestEnable;
				depthStencilStateInfo.depthWriteEnable = depthWriteEnable;
				depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
				depthStencilStateInfo.depthBoundsTestEnable = depthBoundsTestEnable;
				depthStencilStateInfo.stencilTestEnable = stencilTestEnable;

				return depthStencilStateInfo;
			}

			inline VkPipelineColorBlendStateCreateInfo ColorBlendStateInfo(VkBool32 logicOpEnable, VkLogicOp logicOp,
				std::span<const VkPipelineColorBlendAttachmentState> attachments, std::array<float, 4> blendConstants)
			{
				VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
				colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				colorBlendStateInfo.logicOpEnable = logicOpEnable;
				colorBlendStateInfo.logicOp = logicOp;
				colorBlendStateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				colorBlendStateInfo.pAttachments = attachments.data();
				colorBlendStateInfo.blendConstants[0] = blendConstants[0];
				colorBlendStateInfo.blendConstants[1] = blendConstants[1];
				colorBlendStateInfo.blendConstants[2] = blendConstants[2];
				colorBlendStateInfo.blendConstants[3] = blendConstants[3];

				return colorBlendStateInfo;
			}

			inline VkPipelineLayoutCreateInfo PipelineLayoutInfo(std::span<const VkDescriptorSetLayout> setLayouts,
				std::span<const VkPushConstantRange> pushConstantRanges)
			{
				VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
				pipelineLayoutInfo.pSetLayouts = setLayouts.data();
				pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
				pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

				return pipelineLayoutInfo;
			}

			inline VkGraphicsPipelineCreateInfo PipelineInfo(std::span<const VkPipelineShaderStageCreateInfo> shaderStages,
				const VkPipelineVertexInputStateCreateInfo& vertexInputInfo,
				const VkPipelineInputAssemblyStateCreateInfo& inputAssemblyInfo,
				const VkPipelineViewportStateCreateInfo& viewportStateInfo,
				const VkPipelineRasterizationStateCreateInfo& rasterizationInfo,
				const VkPipelineMultisampleStateCreateInfo& multisampleInfo,
				const VkPipelineDepthStencilStateCreateInfo& depthStencilInfo,
				const VkPipelineColorBlendStateCreateInfo& colorBlendingInfo,
				VkPipelineDynamicStateCreateInfo* dynamicStateInfo,
				VkPipelineLayout pipelineLayout, VkRenderPass renderPass, uint32_t subpass, 
				VkPipeline basePipelineHandle, int32_t basePipelineIndex)
			{
				VkGraphicsPipelineCreateInfo pipelineInfo{};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
				pipelineInfo.pStages = shaderStages.data();
				pipelineInfo.pVertexInputState = &vertexInputInfo;
				pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
				pipelineInfo.pViewportState = &viewportStateInfo;
				pipelineInfo.pRasterizationState = &rasterizationInfo;
				pipelineInfo.pMultisampleState = &multisampleInfo;
				pipelineInfo.pDepthStencilState = &depthStencilInfo;
				pipelineInfo.pColorBlendState = &colorBlendingInfo;
				pipelineInfo.pDynamicState = dynamicStateInfo;
				pipelineInfo.layout = pipelineLayout;
				pipelineInfo.renderPass = renderPass;
				pipelineInfo.subpass = subpass;
				pipelineInfo.basePipelineHandle = basePipelineHandle;
				pipelineInfo.basePipelineIndex = basePipelineIndex;

				return pipelineInfo;
			}

			inline VkShaderModuleCreateInfo ShaderModuleInfo(const std::vector<char>& code)
			{
				VkShaderModuleCreateInfo shaderModuleInfo{};
				shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleInfo.codeSize = code.size();
				shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

				return shaderModuleInfo;
			}

			inline VkCommandPoolCreateInfo CommandPoolInfo(uint32_t familyIndex, VkCommandPoolCreateFlags flags)
			{
				VkCommandPoolCreateInfo commandPoolInfo{};
				commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				commandPoolInfo.queueFamilyIndex = familyIndex;
				commandPoolInfo.flags = flags;

				return commandPoolInfo;
			}

			inline VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool commandPool, 
				VkCommandBufferLevel level, uint32_t count)
			{
				VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
				commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				commandBufferAllocateInfo.commandPool = commandPool;
				commandBufferAllocateInfo.level = level;
				commandBufferAllocateInfo.commandBufferCount = count;

				return commandBufferAllocateInfo;
			}

			inline VkSemaphoreCreateInfo SemaphoreInfo()
			{
				VkSemaphoreCreateInfo semaphoreInfo{};
				semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				return semaphoreInfo;
			}

			inline VkFenceCreateInfo FenceInfo(VkFenceCreateFlags flags)
			{
				VkFenceCreateInfo fenceInfo{};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = flags;

				return fenceInfo;
			}

			inline VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
			{
				VkCommandBufferBeginInfo commandBufferBeginInfo{};
				commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				commandBufferBeginInfo.flags = flags;

				return commandBufferBeginInfo;
			}

			inline VkSubmitInfo SubmitInfo(std::span<const VkCommandBuffer> commandBuffers)
			{
				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
				submitInfo.pCommandBuffers = commandBuffers.data();

				return submitInfo;
			}

			inline VkBufferCreateInfo BufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode mode)
			{
				VkBufferCreateInfo bufferCreateInfo{};
				bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferCreateInfo.size = size;
				bufferCreateInfo.usage = usage;
				bufferCreateInfo.sharingMode = mode;

				return bufferCreateInfo;
			}

			inline VkMemoryAllocateInfo MemoryAllocateInfo(VkDeviceSize size, uint32_t memoryTypeIndex)
			{
				VkMemoryAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = size;
				allocInfo.memoryTypeIndex = memoryTypeIndex;

				return allocInfo;
			}

			inline VkImageMemoryBarrier ImageMemoryBarrier(VkImageLayout oldLayout, VkImageLayout newLayout,
				uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkImage image, VkImageAspectFlags aspectMask,
				uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
			{
				VkImageMemoryBarrier imageMemoryBarrier{};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.oldLayout = oldLayout;
				imageMemoryBarrier.newLayout = newLayout;
				imageMemoryBarrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
				imageMemoryBarrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
				imageMemoryBarrier.image = image;
				imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
				imageMemoryBarrier.subresourceRange.baseMipLevel = baseMipLevel;
				imageMemoryBarrier.subresourceRange.levelCount = levelCount;// mipLevels;
				imageMemoryBarrier.subresourceRange.baseArrayLayer = baseArrayLayer;
				imageMemoryBarrier.subresourceRange.layerCount = layerCount;

				return imageMemoryBarrier;
			}
		} // namespace vulkan
	} // namespace Render
} // namespace Eugenix
