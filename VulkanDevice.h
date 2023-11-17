#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <vector>
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	struct QueueFamilyIndices;
	struct SwapChainCreateDetails;

	class VulkanDevice
	{
	public:
		VulkanDevice(bool enableValidationLayer, const VkInstance& instance, VkSurfaceKHR surface,
			const std::vector<const char*>& deviceExtensions, const std::vector<const char*>& validationLayers);

		VkSampleCountFlagBits getMaxUsableSampleCount();
		void pickPhysicalDevice(const VkInstance& instance, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
		bool physicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
		bool physicalDeviceExtensionsCheck(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);

		static QueueFamilyIndices queryQueueFamilyIndices(VkPhysicalDevice device);
		static SwapChainCreateDetails querySwapChainCreateDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

		void createLogicalDevice(bool enableValidationLayer, const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions);

		const VkPhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
		const VkDevice& getLogicalDevice() const { return device; }
		const VkQueue& getGraphicsQueue() { return graphicsQueue; }
		const VkQueue& getPresentQueue() { return presentQueue; }
		const VkSampleCountFlagBits& getMsaaSamples() { return msaaSamples; }

		void destroyDevice();
		~VulkanDevice();

	private:

		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
	};
}

