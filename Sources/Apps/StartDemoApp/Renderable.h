#pragma once

#include <glm/glm.hpp>

#include "Render/Vulkan/VulkanInclude.h"

struct Renderable
{
	void Draw() const
	{
	}

	void UploatToGPU()
	{
	}

	glm::mat4 modelMatrix;
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	uint32_t indexCount;
	VkDescriptorSet descriptorSet;
};