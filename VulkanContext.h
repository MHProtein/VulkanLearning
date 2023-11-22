#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <vector>

#include "VulkanWindow.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <chrono>


namespace my_vulkan
{
	class Object;
	class VulkanInstance;
	class VulkanDevice;
	class VulkanSwapChain;
	class VulkanUniformBuffers;
	class VulkanGraphicsPipeline;
	class VulkanRenderer;
	class Texture;
	class VulkanDescriptors;
	class Mesh;
	class VulkanComputePipeline;
	class Arona;
	class Camera;
	class ImguiAPI;
	class VulkanContext
	{
		friend class ImguiAPI;
		friend class VulkanRenderer;
		friend class Object;
	public:

#ifdef NODEBUG 
		const bool enableValidationLayer = false;
#else
		const bool enableValidationLayer = true;
#endif

		VulkanContext();
		~VulkanContext();
		void run();
		void createWindowSurface();
		void createCommandPool(const VkDevice& device, const VkPhysicalDevice& physicalDevice);

		static void updateImgui(VkCommandBuffer commandBuffer);

		VkCommandPool& getCommandPool() { return commandPool; }


		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor" };
		const std::vector<const char*> deviceExtensions = { "VK_KHR_swapchain", "VK_KHR_shader_non_semantic_info" };

		VulkanWindow wind{ 1920, 1080, "Vulkan" };


		std::shared_ptr<VulkanInstance> instance;
		VkSurfaceKHR surface;
		VkCommandPool commandPool;
		std::shared_ptr<VulkanDevice> device;
		std::shared_ptr<VulkanSwapChain> swapChain;
		std::shared_ptr<VulkanGraphicsPipeline> graphicsPipeline;
		std::shared_ptr<VulkanComputePipeline> computePipeline;


		std::vector<VkBuffer> shaderStorageBuffers;
		std::vector<VkDeviceMemory> shaderStorageBuffersMemory;
		std::shared_ptr<VulkanUniformBuffers> ComputeUniformBuffers;
		std::shared_ptr<VulkanDescriptors> shaderStorageBuffersDescriptors;
		static std::chrono::high_resolution_clock clock;
		const std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

	};


}

