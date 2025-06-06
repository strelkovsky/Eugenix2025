#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <unordered_map>

#include <stb_image.h>
#include <tiny_obj_loader.h>

#include "Render/Vulkan/VulkanApp.h"
#include "Render/Vulkan/VulkanBuffer.h"
#include "Render/Vulkan/VulkanInitializers.h"

#include "Apps/StartDemoApp/Camera.h"
#include "Apps/StartDemoApp/FrameData.h"
#include "Apps/StartDemoApp/Renderable.h"
#include "Apps/StartDemoApp/Vertex.h"
#include "Apps/StartDemoApp/UBO.h"

#include "IO/IO.h"

class StartDemoApp final : public Eugenix::Render::Vulkan::VulkanApp
{
protected:
	bool onInit() override
	{
		createDescriptorSetLayouts();
		createCommandPool();
		initResources();
		createDepthResources();
		createRenderPass();
		createFramebuffers();
		createGraphicsPipeline();
		initRenderables();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObject();

		return true;
	}

	void onCursorMove(double xpos, double ypos) override
	{
		if (_firstMouse)
		{
			_lastX = (float)xpos;
			_lastY = (float)ypos;
			_firstMouse = false;
		}

		float xoffset = (float)xpos - _lastX;
		float yoffset = _lastY - (float)ypos;

		_lastX = (float)xpos;
		_lastY = (float)ypos;

		_camera.processMouse(xoffset, yoffset);
	}

	void onUpdate(float deltaTime) override
	{
		if (KeyPress(GLFW_KEY_W))
			_camera.processKeyboard('W', deltaTime);
		if (KeyPress(GLFW_KEY_S))
			_camera.processKeyboard('S', deltaTime);
		if (KeyPress(GLFW_KEY_A))
			_camera.processKeyboard('A', deltaTime);
		if (KeyPress(GLFW_KEY_D))
			_camera.processKeyboard('D', deltaTime);

		updatePerFrameData(deltaTime);
	}

	void onRender() override
	{
		auto& frame = _frames[_currentFrame];

		vkWaitForFences(_device.Handle(), 1, &frame.inFlight, VK_TRUE, UINT64_MAX);

		uint32_t imageIndex{};
		VkResult acquireResult = vkAcquireNextImageKHR(_device.Handle(), _swapchain.Handle(), UINT64_MAX, frame.imageAvailable, VK_NULL_HANDLE, &imageIndex);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR || _resized)
		{
			_resized = false;
			recreateSwapchain();
			return;
		}
		else if (acquireResult != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to acquire swap chain image!\n");
		}

		updateUniformBuffer(imageIndex);

		vkResetFences(_device.Handle(), 1, &frame.inFlight);
		vkResetCommandBuffer(frame.commandBuffer, 0);
		recordCommandBuffer(frame.commandBuffer, imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { frame.imageAvailable };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frame.commandBuffer;
		VkSemaphore signalSemaphores[] = { frame.renderFinished };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(_device.GraphicsQueue(), 1, &submitInfo, frame.inFlight) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit draw command buffer!\n");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { _swapchain.Handle() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		VkResult result{ vkQueuePresentKHR(_device.PresentQueue(), &presentInfo) };
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _resized)
		{
			_resized = false;
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Rendering failed!\n");
		}

		_currentFrame = (_currentFrame + 1) % Eugenix::Render::Vulkan::Swapchain::MaxFramesInFlight;
	}

	void onCleanup() override
	{
		cleanupSwapchain();

		vkDestroySampler(_device.Handle(), _textureSampler, nullptr);

		vkDestroyImageView(_device.Handle(), _textureImageView, nullptr);
		vkDestroyImage(_device.Handle(), _textureImage, nullptr);
		vkFreeMemory(_device.Handle(), _textureImageMemory, nullptr);

		vkDestroyBuffer(_device.Handle(), _uniformBuffer.buffer, nullptr);
		vkFreeMemory(_device.Handle(), _uniformBuffer.memory, nullptr);

		vkDestroyBuffer(_device.Handle(), _vertexBuffer.buffer, nullptr);
		vkFreeMemory(_device.Handle(), _vertexBuffer.memory, nullptr);

		vkDestroyBuffer(_device.Handle(), _indexBuffer.buffer, nullptr);
		vkFreeMemory(_device.Handle(), _indexBuffer.memory, nullptr);

		vkDestroyDescriptorSetLayout(_device.Handle(), _globalDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(_device.Handle(), _materialDescriptorSetLayout, nullptr);

		std::vector<VkCommandBuffer> commandBuffers;
		for (const auto& frame : _frames)
		{
			commandBuffers.push_back(frame.commandBuffer);
		}
		vkFreeCommandBuffers(_device.Handle(), _commandPool,
			static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyCommandPool(_device.Handle(), _commandPool, nullptr);

		glfwDestroyWindow(_window);
		glfwTerminate();
	}

private:
	VkRenderPass _renderPass{ VK_NULL_HANDLE };
	VkPipelineLayout _pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline _graphicsPipeline{ VK_NULL_HANDLE };

	std::vector<VkFramebuffer> _swapchainFramebuffers;
	VkCommandPool _commandPool{ VK_NULL_HANDLE };

	std::array<FrameData, Eugenix::Render::Vulkan::Swapchain::MaxFramesInFlight> _frames;
	size_t _currentFrame{ 0 };

	VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
	VkDescriptorSetLayout _globalDescriptorSetLayout;  // set = 0 (view/proj)
	VkDescriptorSetLayout _materialDescriptorSetLayout;  // set = 1 (sampler)
	VkDescriptorSet _globalDescriptorSet;
	VkDescriptorSet _materialDescriptorSet;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Eugenix::Render::Vulkan::Buffer _vertexBuffer;
	Eugenix::Render::Vulkan::Buffer _indexBuffer;

	Eugenix::Render::Vulkan::Buffer _uniformBuffer;

	VkImage _textureImage;
	VkDeviceMemory _textureImageMemory;
	VkImageView _textureImageView;
	VkSampler _textureSampler;

	VkImage _depthImage;
	VkDeviceMemory _depthImageMemory;
	VkImageView _depthImageView;

	double _lastTime;
	int _frameCount{ 0 };

	Camera _camera;
	float _lastX = 0.0f, _lastY = 0.0f;
	bool _firstMouse = true;
	float _deltaTime = 0.0f, _lastFrame = 0.0f;

	std::vector<Renderable> _renderables;

	Eugenix::Render::Vulkan::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		Eugenix::Render::Vulkan::QueueFamilyIndices indices;

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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface.Handle(), &presentSupport);
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

	void createFramebuffers()
	{
		_swapchainFramebuffers.resize(_swapchain.ImageViews().size());

		for (size_t i = 0; i < _swapchain.Images().size(); ++i)
		{
			std::array<VkImageView, 2> attachments = { _swapchain.ImageViews()[i], _depthImageView };

			VkFramebufferCreateInfo framebufferInfo = Eugenix::Render::Vulkan::FrameBufferInfo(_renderPass,
				attachments, _swapchain.Extent(), 1);

			if (vkCreateFramebuffer(_device.Handle(), &framebufferInfo, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS)
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

		vkDeviceWaitIdle(_device.Handle());

		cleanupSwapchain();

		_swapchain.Create(_adapter, _surface, _device, _window);
		createDepthResources();
		createRenderPass();
		createFramebuffers();
		createGraphicsPipeline();
		createDescriptorPool();
		createDescriptorSets();

		createCommandBuffers();
		createSyncObject();

		_currentFrame = 0;
	}

	void createRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = _swapchain.Format();
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

		if (vkCreateRenderPass(_device.Handle(), &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass\n");
		}
	}

	void createDescriptorSetLayouts()
	{
		// Ubo
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = Eugenix::Render::Vulkan::DescriptorSetLayoutInfo(bindings);
		if (vkCreateDescriptorSetLayout(_device.Handle(), &layoutInfo, nullptr, &_globalDescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!\n");
		}

		// Sampler
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		bindings = { samplerLayoutBinding };

		layoutInfo = Eugenix::Render::Vulkan::DescriptorSetLayoutInfo(bindings);
		if (vkCreateDescriptorSetLayout(_device.Handle(), &layoutInfo, nullptr, &_materialDescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!\n");
		}
	}

	void createDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};

		// ubo pool size
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;

		// sampler pool size
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 100;

		VkDescriptorPoolCreateInfo poolInfo = Eugenix::Render::Vulkan::DescriptorPoolInfo(
			poolSizes, static_cast<uint32_t>(100 + 1));

		if (vkCreateDescriptorPool(_device.Handle(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool!\n");
		}
	}

	void createDescriptorSets()
	{
		std::array<VkDescriptorSetLayout, 1> setLayouts = { _globalDescriptorSetLayout };

		VkDescriptorSetAllocateInfo allocInfo = Eugenix::Render::Vulkan::DescriptorSetAllocateInfo(_descriptorPool,
			1, setLayouts);

		if (vkAllocateDescriptorSets(_device.Handle(), &allocInfo, &_globalDescriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!\n");
		}

		setLayouts = { _materialDescriptorSetLayout };

		VkDescriptorSetAllocateInfo samplerAllocInfo = Eugenix::Render::Vulkan::DescriptorSetAllocateInfo(_descriptorPool,
			1, setLayouts);

		if (vkAllocateDescriptorSets(_device.Handle(), &samplerAllocInfo, &_materialDescriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!\n");
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = _uniformBuffer.buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = _textureImageView;
		imageInfo.sampler = _textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0] = Eugenix::Render::Vulkan::WriteDescriptorSet(_globalDescriptorSet, 0, 0, 1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfo);
		descriptorWrites[1] = Eugenix::Render::Vulkan::WriteDescriptorSet(_materialDescriptorSet, 1, 0, 1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageInfo);

		vkUpdateDescriptorSets(_device.Handle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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
		viewport.width = static_cast<float>(_swapchain.Extent().width);
		viewport.height = static_cast<float>(_swapchain.Extent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		std::array<VkViewport, 1> viewports = { viewport };

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = _swapchain.Extent();

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

		std::array<VkDescriptorSetLayout, 2> setLayouts =
		{
			_globalDescriptorSetLayout,
			_materialDescriptorSetLayout
		};
		std::array<VkPushConstantRange, 1> pushConstantRanges = { pushConstantRange };
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Eugenix::Render::Vulkan::PipelineLayoutInfo(setLayouts,
			pushConstantRanges);

		if (vkCreatePipelineLayout(_device.Handle(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout!\n");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = Eugenix::Render::Vulkan::PipelineInfo(shaderStages,
			vertexInputInfo, inputAssembly, viewportState, rasterizer, multisampling, depthStencilAttachment,
			colorBlending, nullptr, _pipelineLayout, _renderPass, 0, VK_NULL_HANDLE, -1);

		if (vkCreateGraphicsPipelines(_device.Handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!\n");
		}

		vkDestroyShaderModule(_device.Handle(), vertShaderModule, nullptr);
		vkDestroyShaderModule(_device.Handle(), fragShaderModule, nullptr);
	}

	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = Eugenix::Render::Vulkan::ShaderModuleInfo(code);

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(_device.Handle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
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
		obj1.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
		obj1.modelMatrix = glm::scale(obj1.modelMatrix, glm::vec3(0.5f));
		obj1.modelMatrix = glm::rotate(obj1.modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		obj1.modelMatrix = glm::rotate(obj1.modelMatrix, glm::radians(-90.0f), glm::vec3(0, 0, 1));
		obj1.vertexBuffer = _vertexBuffer.buffer;
		obj1.indexBuffer = _indexBuffer.buffer;
		obj1.indexCount = static_cast<uint32_t>(indices.size());
		obj1.descriptorSet = _globalDescriptorSet;

		Renderable obj2;
		obj2.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		obj2.modelMatrix = glm::scale(obj2.modelMatrix, glm::vec3(0.5f));
		obj2.modelMatrix = glm::rotate(obj2.modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		obj2.modelMatrix = glm::rotate(obj2.modelMatrix, glm::radians(-90.0f), glm::vec3(0, 0, 1));
		obj2.vertexBuffer = _vertexBuffer.buffer;
		obj2.indexBuffer = _indexBuffer.buffer;
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
		Eugenix::Render::Vulkan::QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_adapter.Handle());

		VkCommandPoolCreateInfo poolInfo = Eugenix::Render::Vulkan::CommandPoolInfo(queueFamilyIndices.graphicsFamily.value(),
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		if (vkCreateCommandPool(_device.Handle(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!\n");
		}
	}

	void createCommandBuffers()
	{
		std::vector<VkCommandBuffer> commandBuffers(Eugenix::Render::Vulkan::Swapchain::MaxFramesInFlight);

		VkCommandBufferAllocateInfo allocInfo = Eugenix::Render::Vulkan::CommandBufferAllocateInfo(_commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY, Eugenix::Render::Vulkan::Swapchain::MaxFramesInFlight);

		if (vkAllocateCommandBuffers(_device.Handle(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!\n");
		}

		for (size_t i = 0; i < Eugenix::Render::Vulkan::Swapchain::MaxFramesInFlight; ++i)
		{
			_frames[i].commandBuffer = commandBuffers[i];
		}
	}

	void createSyncObject()
	{
		VkSemaphoreCreateInfo semaphoreInfo = Eugenix::Render::Vulkan::SemaphoreInfo();
		VkFenceCreateInfo fenceInfo = Eugenix::Render::Vulkan::FenceInfo(VK_FENCE_CREATE_SIGNALED_BIT);

		for (auto& frame : _frames)
		{
			if (vkCreateSemaphore(_device.Handle(), &semaphoreInfo, nullptr, &frame.imageAvailable) != VK_SUCCESS ||
				vkCreateSemaphore(_device.Handle(), &semaphoreInfo, nullptr, &frame.renderFinished) != VK_SUCCESS ||
				vkCreateFence(_device.Handle(), &fenceInfo, nullptr, &frame.inFlight) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create sync objects for a frame!\n");
			}
		}
	}

	VkCommandBuffer beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo = Eugenix::Render::Vulkan::CommandBufferAllocateInfo(_commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(_device.Handle(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = Eugenix::Render::Vulkan::CommandBufferBeginInfo(
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkCommandBuffer commandBuffers[] = { commandBuffer };
		VkSubmitInfo submitInfo = Eugenix::Render::Vulkan::SubmitInfo(commandBuffers);

		vkQueueSubmit(_device.GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(_device.GraphicsQueue());

		vkFreeCommandBuffers(_device.Handle(), _commandPool, 1, &commandBuffer);
	}

	void createVertexBuffer()
	{
		VkDeviceSize size = sizeof(Vertex) * vertices.size();

		Eugenix::Render::Vulkan::Buffer stagingBuffer;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer);

		void* data;
		vkMapMemory(_device.Handle(), stagingBuffer.memory, 0, size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(size));
		vkUnmapMemory(_device.Handle(), stagingBuffer.memory);

		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			_vertexBuffer);

		copyBuffer(stagingBuffer.buffer, _vertexBuffer.buffer, size);
		vkDestroyBuffer(_device.Handle(), stagingBuffer.buffer, nullptr);
		vkFreeMemory(_device.Handle(), stagingBuffer.memory, nullptr);
	}

	void createIndexBuffer()
	{
		VkDeviceSize size = sizeof(uint32_t) * indices.size();

		Eugenix::Render::Vulkan::Buffer stagingBuffer;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer);

		void* data;
		vkMapMemory(_device.Handle(), stagingBuffer.memory, 0, size, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(size));
		vkUnmapMemory(_device.Handle(), stagingBuffer.memory);

		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			_indexBuffer);

		copyBuffer(stagingBuffer.buffer, _indexBuffer.buffer, size);
		vkDestroyBuffer(_device.Handle(), stagingBuffer.buffer, nullptr);
		vkFreeMemory(_device.Handle(), stagingBuffer.memory, nullptr);
	}

	void createUniformBuffers()
	{
		// Camera UBO
		VkDeviceSize size = sizeof(UniformBufferObject);

		createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			_uniformBuffer);
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Eugenix::Render::Vulkan::Buffer& buffer)
	{
		VkBufferCreateInfo bufferInfo = Eugenix::Render::Vulkan::BufferCreateInfo(size, usage, VK_SHARING_MODE_EXCLUSIVE);
		if (vkCreateBuffer(_device.Handle(), &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device.Handle(), buffer.buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = Eugenix::Render::Vulkan::MemoryAllocateInfo(memRequirements.size,
			findMemoryType(memRequirements.memoryTypeBits, properties));

		if (vkAllocateMemory(_device.Handle(), &allocInfo, nullptr, &buffer.memory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory!\n");
		}

		vkBindBufferMemory(_device.Handle(), buffer.buffer, buffer.memory, 0);
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
		vkGetPhysicalDeviceMemoryProperties(_adapter.Handle(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type\n");
	}

	void initResources()
	{
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		createUniformBuffers();
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

		Eugenix::Render::Vulkan::Buffer stagingBuffer;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer);

		void* data;
		vkMapMemory(_device.Handle(), stagingBuffer.memory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<uint32_t>(imageSize));
		vkUnmapMemory(_device.Handle(), stagingBuffer.memory);
		stbi_image_free(pixels);

		createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _textureImage, _textureImageMemory);

		transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer.buffer, _textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(_device.Handle(), stagingBuffer.buffer, nullptr);
		vkFreeMemory(_device.Handle(), stagingBuffer.memory, nullptr);
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
		_textureImageView = _device.CreateImageView(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
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
		if (vkCreateImage(_device.Handle(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image!\n");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(_device.Handle(), image, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(_device.Handle(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory!\n");
		}

		vkBindImageMemory(_device.Handle(), image, imageMemory, 0);
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

		if (vkCreateSampler(_device.Handle(), &samplerInfo, nullptr, &_textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler!\n");
		}
	}

	void createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();

		createImage(_swapchain.Extent().width, _swapchain.Extent().height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
		_depthImageView = _device.CreateImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	VkFormat findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT }, // ������ �������� � ������� ������������
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}


	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(_adapter.Handle(), format, &props);

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
		renderPassInfo.renderArea.extent = _swapchain.Extent();
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.5f, 0.0f, 0.25f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

		VkDeviceSize offsets[] = { 0 };

		for (const auto& renderable : _renderables)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &renderable.vertexBuffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, renderable.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &renderable.modelMatrix);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_globalDescriptorSet, 0, nullptr);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_materialDescriptorSet, 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(renderable.indexCount), 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!\n");
		}
	}

	void updatePerFrameData(float deltaTime)
	{
		_renderables[0].modelMatrix = glm::rotate(_renderables[0].modelMatrix, deltaTime, glm::vec3(0, 0, 1));
		_renderables[1].modelMatrix = glm::rotate(_renderables[1].modelMatrix, deltaTime, glm::vec3(0, 0, 1));
	}

	void updateUniformBuffer(uint32_t currentImage)
	{
		UniformBufferObject ubo{};
		ubo.view = _camera.getViewMatrix();
		ubo.proj = glm::perspective(glm::radians(45.0f), float(_swapchain.Extent().width) / float(_swapchain.Extent().height), 0.1f, 100.0f);
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(_device.Handle(), _uniformBuffer.memory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(_device.Handle(), _uniformBuffer.memory);
	}

	void cleanupSwapchain()
	{
		for (auto& frame : _frames)
		{
			vkDestroySemaphore(_device.Handle(), frame.renderFinished, nullptr);
			vkDestroySemaphore(_device.Handle(), frame.imageAvailable, nullptr);
			vkDestroyFence(_device.Handle(), frame.inFlight, nullptr);
		}

		vkDestroyImageView(_device.Handle(), _depthImageView, nullptr);
		vkDestroyImage(_device.Handle(), _depthImage, nullptr);
		vkFreeMemory(_device.Handle(), _depthImageMemory, nullptr);

		vkDestroyDescriptorPool(_device.Handle(), _descriptorPool, nullptr);

		for (auto framebuffer : _swapchainFramebuffers)
		{
			vkDestroyFramebuffer(_device.Handle(), framebuffer, nullptr);
		}

		vkDestroyPipeline(_device.Handle(), _graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(_device.Handle(), _pipelineLayout, nullptr);
		vkDestroyRenderPass(_device.Handle(), _renderPass, nullptr);

		_swapchain.Destroy(_device.Handle());
	}
};