#include "VulkanImage.h"
#include "VulkanDevice.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"
#include <stdexcept>

my_vulkan::VulkanImage::VulkanImage(const std::shared_ptr<my_vulkan::VulkanDevice>& device, uint32_t width,
	uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, VkImageType imageType, VkFormat format,
	VkImageTiling tilingMode, VkImageLayout initialLayout, VkImageUsageFlags usage, VkSharingMode sharingMode,
	VkSampleCountFlagBits sampleCount, VkMemoryPropertyFlags memoryProperties, VkImageAspectFlags aspectFlags)
{
	createImage(device, width, height, depth, mipLevels, arrayLayers, imageType, format, tilingMode, initialLayout, usage, sharingMode, sampleCount, memoryProperties);
	createImageView(device->getLogicalDevice(), aspectFlags, mipLevels);
}

void my_vulkan::VulkanImage::createImage(const std::shared_ptr<my_vulkan::VulkanDevice>& device, uint32_t width, uint32_t height, uint32_t depth,
                                         uint32_t mipLevels, uint32_t araryLayers, VkImageType imageType, VkFormat format, VkImageTiling tilingMode, VkImageLayout initialLayout,
                                         VkImageUsageFlags usage, VkSharingMode sharingMode, VkSampleCountFlagBits sampleCount, VkMemoryPropertyFlags memoryProperties)
{ 
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = imageType;
	imageInfo.extent.height = height;
	imageInfo.extent.width = width;
	imageInfo.extent.depth = depth;
	imageInfo.sharingMode = sharingMode;
	imageInfo.samples = sampleCount;
	imageInfo.arrayLayers = araryLayers;
	imageInfo.format = format;
	imageInfo.initialLayout = initialLayout;
	imageInfo.mipLevels = mipLevels;
	imageInfo.tiling = tilingMode;
	imageInfo.usage = usage;

	this->format = format;
	this->layout = initialLayout;

	if (vkCreateImage(device->getLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture image!");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->getLogicalDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VulkanUtils::findMemoryType(device->getPhysicalDevice(), memoryProperties, memRequirements.memoryTypeBits);

	if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device->getLogicalDevice(), image, imageMemory, 0);
}

void my_vulkan::VulkanImage::createImageView(const VkDevice& device, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}

void my_vulkan::VulkanImage::transitionImageLayout(const std::shared_ptr<my_vulkan::VulkanDevice>& device, VkCommandPool& commandPool, VkImageLayout newLayout, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = VulkanUtils::beginSingleTimeCommand(device->getLogicalDevice(), commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.image = image;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.oldLayout = layout;
	barrier.newLayout = newLayout;

	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (VulkanUtils::hasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	if(layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

		srcStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if(layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::invalid_argument("old layout to new layout not supported!");
	}

	layout = newLayout;

	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 
		0, 0, 
		nullptr, 0,
		nullptr, 
		1, &barrier);

	VulkanUtils::endSingleTimeCommands(device->getLogicalDevice(), commandBuffer, commandPool, device->getGraphicsQueue());
}

void my_vulkan::VulkanImage::copyBufferToImage(const std::shared_ptr<my_vulkan::VulkanDevice>& device, VkCommandPool& commandPool, VkBuffer& buffer, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = VulkanUtils::beginSingleTimeCommand(device->getLogicalDevice(), commandPool);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferImageHeight = 0;
	region.bufferRowLength = 0;
	region.imageExtent = { width, height, 1 };
	region.imageOffset = { 0, 0 };
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.mipLevel = 0;
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	VulkanUtils::endSingleTimeCommands(device->getLogicalDevice(), commandBuffer, commandPool, device->getGraphicsQueue());
}

void my_vulkan::VulkanImage::destroyImage(const VkDevice& device)
{
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
	vkDestroyImageView(device, imageView, nullptr);
}
