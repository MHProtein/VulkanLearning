#include "VulkanDepthResources.h"

#include <stdexcept>

#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanUtils.h"

my_vulkan::VulkanDepthResources::VulkanDepthResources(const std::shared_ptr<VulkanDevice>& device, VkExtent2D extent, VkCommandPool& commandPool)
{
	createDepthBuffer(device, extent, commandPool);
}

void my_vulkan::VulkanDepthResources::createDepthBuffer(const std::shared_ptr<VulkanDevice>& device, VkExtent2D extent, VkCommandPool& commandPool)
{
	VkFormat format = findDepthFormat(device);

	image = std::make_shared<VulkanImage>(device, extent.width, extent.height, 1, 1, 1, VkImageType::VK_IMAGE_TYPE_2D, 
		format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
		VK_SHARING_MODE_EXCLUSIVE, device->getMaxUsableSampleCount(), VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

	image->transitionImageLayout(device, commandPool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkFormat my_vulkan::VulkanDepthResources::findDepthFormat(const std::shared_ptr<VulkanDevice>& device)
{
	return VulkanUtils::findSupportFormat(device, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkImage& my_vulkan::VulkanDepthResources::getImage()
{
	return image->getImage();
}

VkDeviceMemory& my_vulkan::VulkanDepthResources::getImageMemory()
{
	return image->getImageMemory();
}

VkFormat& my_vulkan::VulkanDepthResources::getImageFormat()
{
	return image->getImageFormat();
}

VkImageLayout& my_vulkan::VulkanDepthResources::getImageCurrentLayout()
{
	return image->getImageCurrentLayout();
}

VkImageView& my_vulkan::VulkanDepthResources::getImageView()
{
	return image->getImageView();
}
