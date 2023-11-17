#pragma once
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	class VulkanDevice;
	class VulkanSwapChain;
	class VulkanDescriptors;


	class VulkanGraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<VulkanSwapChain>& swapChain, VkCommandPool& commandPool);

		void createRenderPass(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<VulkanSwapChain>& swapChain, VkCommandPool& commandPool);


		void createGraphicsPipeline(const VkDevice& device, const VkExtent2D& swapChainExtent, VkSampleCountFlagBits msaaCount);

		const VkRenderPass& getRenderPass() const { return renderPass; }
		const VkPipelineLayout& getPipelineLayout() const { return graphicsPipelineLayout; }
		const VkPipeline& getGraphicsPipeline() const { return graphicsPipeline; }

		void destroyGraphicsPipeline(const VkDevice& device);

	private:

		VkRenderPass renderPass;
		VkPipelineLayout graphicsPipelineLayout;
		VkPipeline graphicsPipeline;
	
	};
}
