#pragma once
#include <memory>

#include "vulkan/vulkan.h"



namespace my_vulkan
{
	class VulkanDevice;

	class VulkanComputePipeline
	{
	public:

		VulkanComputePipeline(const std::shared_ptr<VulkanDevice>& device);

		void createComputePipeline(const std::shared_ptr<VulkanDevice>& device);

		VkPipeline getComputePipeline() { return computePipeline; }
		VkPipelineLayout& getComputePipelineLayout() { return computePipelineLayout; }

	private:
		VkPipeline computePipeline;
		VkPipelineLayout computePipelineLayout;
	};

}

