#include "VulkanSwapChain.h"
#include "VulkanUtils.h"
#include <algorithm>

#include "VulkanDevice.h"
#include <stdexcept>


my_vulkan::VulkanSwapChain::VulkanSwapChain(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkSurfaceKHR& surface, GLFWwindow* window)
{
	createSwapChain(physicalDevice, device, surface, window);
	createImageViews(device);
}

VkExtent2D my_vulkan::VulkanSwapChain::chooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != (std::numeric_limits<uint64_t>::max)())
		return capabilities.currentExtent;

	VkExtent2D actualExtent;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	actualExtent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

VkSurfaceFormatKHR my_vulkan::VulkanSwapChain::chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	for (auto format : formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SNORM && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			return format;
	}
	return formats[0];
}

VkPresentModeKHR my_vulkan::VulkanSwapChain::choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
{
	for (auto presentMode : presentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

void my_vulkan::VulkanSwapChain::createSwapChain(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkSurfaceKHR& surface, GLFWwindow* window)
{
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	auto details = VulkanDevice::querySwapChainCreateDetails(physicalDevice, surface);

	auto extent = chooseSwapChainExtent(details.capabilities, window);
	auto format = chooseSwapChainFormat(details.formats);
	auto presentMode = choosePresentMode(details.presentModes);

	createInfo.imageExtent = extent;
	createInfo.imageFormat = format.format;
	createInfo.imageColorSpace = format.colorSpace;
	createInfo.presentMode = presentMode;

	createInfo.clipped = VK_TRUE;
	createInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	createInfo.surface = surface;

	auto indices = VulkanDevice::queryQueueFamilyIndices(physicalDevice);
	uint32_t queueIndices[] = { indices.graphicsAndComputeQueue.value(), indices.presentQueue.value() };
	if (indices.graphicsAndComputeQueue.value() != indices.presentQueue.value())
	{
		createInfo.imageSharingMode = VkSharingMode::VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueIndices;
	}
	else
	{
		createInfo.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	uint32_t imageCount = details.capabilities.minImageCount;
	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
		imageCount = details.capabilities.maxImageCount;
	createInfo.minImageCount = imageCount;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw std::runtime_error("failed to create swap chain!");

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainExtent = extent;
	swapChainFormat = format;
	swapChainPresentMode = presentMode;
}

void my_vulkan::VulkanSwapChain::createImageViews(const VkDevice& device)
{
	imageViews.resize(swapChainImages.size());
	int i = 0;
	for (auto& view : imageViews)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.format = swapChainFormat.format;
		createInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A;
		createInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R;
		createInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G;
		createInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B;
		createInfo.image = swapChainImages[i++];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		createInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.layerCount = 1;
		createInfo.subresourceRange.levelCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &view) != VK_SUCCESS)
			throw std::runtime_error("failed to create!");
	}
}

void my_vulkan::VulkanSwapChain::DestroySwapChain(const VkDevice& device)
{
	vkDestroySwapchainKHR(device, swapChain, nullptr);
	for (auto& imageView : imageViews)
		vkDestroyImageView(device, imageView, nullptr);
	for (auto& image : swapChainImages)
		vkDestroyImage(device, image, nullptr);
}

my_vulkan::VulkanSwapChain::~VulkanSwapChain()
{

	
}
