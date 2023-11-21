#pragma once
#include <memory>
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
#include "glm/glm.hpp"

namespace my_vulkan
{
	struct UniformBufferObject;
	enum class VulkanUBOFor;
	class VulkanGraphicsPipeline;

	class VulkanUniformBuffers
	{
		friend class VulkanGraphicsPipeline;
	public:
		VulkanUniformBuffers(const std::shared_ptr<VulkanDevice>& device, VulkanUBOFor type);
		
		void createUniformBuffers(const std::shared_ptr<VulkanDevice>& device, VulkanUBOFor type);

		void updateUniformBuffer(uint32_t currentImage, UniformBufferObject* ubo);
	
		std::vector<VkBuffer>& getUniformBuffers() { return uniformBuffers; }
		std::vector<VkDeviceMemory>& getUniformBuffersMemory() { return uniformBuffersMemory; }
		std::vector<void*>& getUniformBuffersMapped() { return uniformBuffersMapped; }

		void DestroyVulkanUniformBuffers(const VkDevice& device);

		~VulkanUniformBuffers();

	private:
		VulkanUBOFor type;
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;
	};


}

