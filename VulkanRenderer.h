#pragma once
#define GLM_FORCE_RADIANS
#include <memory>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>

#include "VulkanWindow.h"

namespace my_vulkan
{
	class ImguiAPI;
	class VulkanContext;
	class VulkanWindow;
	class VulkanComputePipeline;
	class VulkanImage;
	class Object;
	class VulkanDepthResources;
	struct Vertex;
	class VulkanDescriptors;
	class VulkanDevice;
	class VulkanUniformBuffers;
	class VulkanSwapChain;
	class VulkanGraphicsPipeline;

	class VulkanRenderer
	{
	public:
		VulkanRenderer(my_vulkan::VulkanContext* context);

		void createFramebuffers(const VkDevice& device, const std::shared_ptr<VulkanSwapChain> swapChain, const VkRenderPass& renderPass);
		void createCommandBuffer(const VkDevice& device, VkCommandPool& commandPool);
		void createSynchronizationObjects(const VkDevice& device);

		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::shared_ptr<VulkanGraphicsPipeline>& pipeline,
			const VkExtent2D& swapChainExtent, ImguiAPI* imgui, const std::vector<std::shared_ptr<Object>>& objects);

		void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, const std::shared_ptr<VulkanComputePipeline>& computePipeline,
			const std::shared_ptr<VulkanDescriptors>& descriptors);

		void draw(my_vulkan::VulkanContext* context, ImguiAPI* imgui, const std::vector<std::shared_ptr<Object>>& objects);

		void recreateSwapChain(std::shared_ptr<VulkanSwapChain> swapChain, GLFWwindow* window, const std::shared_ptr<VulkanDevice>& device, 
			const VkSurfaceKHR& surface, const VkRenderPass& renderPass, VkCommandPool& commandPool);

		uint32_t getCurrentFrame() const { return currentFrame; }

		void destroyRenderer(const VkDevice& device);
		~VulkanRenderer();

	private:
		const uint32_t maxRenderImages;
		std::vector<VkFramebuffer> frameBuffers;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkCommandBuffer> computeCommandBuffers;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		uint32_t currentFrame;

		std::shared_ptr<VulkanImage> colorRecources;
		std::shared_ptr<VulkanDepthResources> depthResources;
		std::array<VkClearValue, 2> clearValues;

	
	};
}

