#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkSurfaceKHR& surface, GLFWwindow* window);

		VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow* window);
		VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);
		void createSwapChain(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkSurfaceKHR& surface, GLFWwindow* window);

		void createImageViews(const VkDevice& device);

		const VkSwapchainKHR& getSwapChain() const { return swapChain; }
		const VkExtent2D& getSwapChainExtent() const { return swapChainExtent; }
		const VkSurfaceFormatKHR& getSwapChainFormat() const { return swapChainFormat; }
		const VkPresentModeKHR& getSwapChainPresentMode() const { return swapChainPresentMode; }
		const std::vector<VkImage>& getSwapChainImages() const { return swapChainImages; }
		const std::vector<VkImageView>& getImageViews() const { return imageViews; }

		void recreateSwapChain(GLFWwindow* window, const VkDevice& device, const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
		{
			int width = 0, height = 0;
			glfwGetFramebufferSize(window, &width, &height);
			while (width == 0 || height == 0)
			{
				glfwGetFramebufferSize(window, &width, &height);
				glfwWaitEvents();
			}

			for (size_t i = 0; i < imageViews.size(); i++) {
				vkDestroyImageView(device, imageViews[i], nullptr);
			}
			vkDestroySwapchainKHR(device, swapChain, nullptr);

			vkDeviceWaitIdle(device);
			createSwapChain(physicalDevice, device, surface, window);
			createImageViews(device);
		}

		void DestroySwapChain(const VkDevice& device);
		~VulkanSwapChain();

	private:
		VkSwapchainKHR swapChain;
		VkExtent2D swapChainExtent;
		VkSurfaceFormatKHR swapChainFormat;
		VkPresentModeKHR swapChainPresentMode;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> imageViews;
	};
}
