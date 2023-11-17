#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <vector>

#include "VulkanWindow.h"
#include <vulkan/vulkan.h>
#include <memory>




namespace my_vulkan
{
	class Object;
	class Object;
	class VulkanInstance;
	class VulkanDevice;
	class VulkanSwapChain;
	class VulkanUniformBuffers;
	class VulkanGraphicsPipeline;
	class VulkanRenderer;
	class Texture;
	class VulkanDescriptors;
	class Model;
	class VulkanComputePipeline;

	class FirstApp
	{
	public:
		const uint32_t WIDTH = 1920;
		const uint32_t HEIGHT = 1080;

		const std::vector<std::string> aronaTexturePaths = {
			//"Models/arona/Arona_Body.png",
			//"Models/arona/Arona_Eye.png" ,
			//"Models/arona/Arona_Hair.png" ,
			//"Models/arona/Arona_Halo.png",
			//"Models/arona/Arona_Eyebrow.png",
			//"Models/arona/Arona_Face.png"
			"D:/GitHub/GAMES202/homework0/assets/mary/MC003_Kozakura_Mari.png"
		};

		const std::vector<std::string> aronaModelPaths = {
			//"Models/arona/arona_body.obj",
			//"Models/arona/arona_eye.obj" ,
			//"Models/arona/arona_hair.obj" ,
			//"Models/arona/arona_halo.obj",
			//"Models/arona/arona_eyebrow.obj",
			//"Models/arona/arona_face.obj"
			"D:/GitHub/GAMES202/homework0/assets/mary/Marry.obj"
		};

#ifdef NODEBUG 
		const bool enableValidationLayer = false;
#else
		const bool enableValidationLayer = true;
#endif

		FirstApp();
		~FirstApp();
		void run();
		void initImgui();
		void createWindowSurface();
		void createCommandPool(const VkDevice& device, const VkPhysicalDevice& physicalDevice);

		void createSSBO(const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool);

		VkCommandPool& getCommandPool() { return commandPool; }

		//const std::vector<Vertex> vertices = {
		//	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		//	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		//	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		//	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		//	{ {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		//	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		//	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		//	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		//};

		//const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor" };
		const std::vector<const char*> deviceExtensions = { "VK_KHR_swapchain" };
	private:
		VulkanWindow wind{WIDTH, HEIGHT, "MyVulkan"};
		std::shared_ptr<VulkanInstance> instance;
		VkSurfaceKHR surface;
		VkCommandPool commandPool;
		std::shared_ptr<VulkanDevice> device;
		std::shared_ptr<VulkanSwapChain> swapChain;
		std::shared_ptr<VulkanGraphicsPipeline> graphicsPipeline;
		std::shared_ptr<VulkanComputePipeline> computePipeline;
		std::shared_ptr<VulkanRenderer> renderer;
		std::shared_ptr<Object> arona;

		std::vector<VkBuffer> shaderStorageBuffers;
		std::vector<VkDeviceMemory> shaderStorageBuffersMemory;
		std::shared_ptr<VulkanUniformBuffers> ComputeUniformBuffers;
		std::shared_ptr<VulkanDescriptors> shaderStorageBuffersDescriptors;
	};


}

