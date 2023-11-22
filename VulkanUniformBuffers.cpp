#define GLM_FORCE_RADIANS

#include "VulkanUniformBuffers.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "imgui.h"
#include "Vertex.h"
#include "VulkanUtils.h"

my_vulkan::VulkanUniformBuffers::VulkanUniformBuffers(const std::shared_ptr<VulkanDevice>& device, VulkanUBOFor type) : type(type)
{
	createUniformBuffers(device, type);
}

void my_vulkan::VulkanUniformBuffers::createUniformBuffers(const std::shared_ptr<VulkanDevice>& device, VulkanUBOFor type)
{
	bufferSize;
	switch (type)
	{
	case VulkanUBOFor::VERTEX_SHADER :
		bufferSize = sizeof(VertexUniformBufferObject);
		break;
	case VulkanUBOFor::FRAGMENT_SHADER:
		bufferSize = sizeof(FragmentUniformBufferObject);
		break;
	case VulkanUBOFor::COMPUTE_SHADER:
		bufferSize = sizeof(ParameterUBO);
		break;
	}

	uniformBuffers.resize(MAX_RENDER_IMAGES);
	uniformBuffersMemory.resize(MAX_RENDER_IMAGES);
	uniformBuffersMapped.resize(MAX_RENDER_IMAGES);

	for (size_t i = 0; i < MAX_RENDER_IMAGES; ++i)
	{
		VulkanUtils::createBuffer(device, uniformBuffers[i], uniformBuffersMemory[i], bufferSize, 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		vkMapMemory(device->getLogicalDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
	//The buffer stays mapped to this pointer for the application's whole lifetime, this technique is called "persistent mapping"
	//Not having to map the buffer every time we need to update it increases performances
}

void my_vulkan::VulkanUniformBuffers::updateUniformBuffer(uint32_t currentImage, UniformBufferObject* ubo)
{
	memcpy(uniformBuffersMapped[currentImage], reinterpret_cast<FragmentUniformBufferObject*>(ubo), bufferSize);
}

void my_vulkan::VulkanUniformBuffers::DestroyVulkanUniformBuffers(const VkDevice& device)
{
	for (size_t i = 0; i < MAX_RENDER_IMAGES; i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}
}

my_vulkan::VulkanUniformBuffers::~VulkanUniformBuffers()
{

}
