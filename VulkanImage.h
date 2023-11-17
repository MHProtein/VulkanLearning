#pragma once
#include <memory>
#include <vector>

#include "vulkan/vulkan.h"

namespace my_vulkan
{
	class VulkanDevice;

	class VulkanImage
	{
	public:
		VulkanImage(const std::shared_ptr<VulkanDevice>& device, uint32_t width, uint32_t height, uint32_t depth,
			uint32_t mipLevels, uint32_t arrayLayers, VkImageType imageType, VkFormat format, VkImageTiling tilingMode, VkImageLayout initialLayout, VkImageUsageFlags usage,
			VkSharingMode sharingMode, VkSampleCountFlagBits sampleCount, VkMemoryPropertyFlags memoryProperties, VkImageAspectFlags aspectFlags);

		void createImage(const std::shared_ptr<my_vulkan::VulkanDevice>& device, uint32_t width, uint32_t height, uint32_t depth,
			uint32_t mipLevels, uint32_t araryLayers, VkImageType imageType, VkFormat format, VkImageTiling tilingMode, VkImageLayout initialLayout, VkImageUsageFlags usage,
			VkSharingMode sharingMode, VkSampleCountFlagBits sampleCount, VkMemoryPropertyFlags memoryProperties);

		void createImageView(const VkDevice& device, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

		void transitionImageLayout(const std::shared_ptr<my_vulkan::VulkanDevice>& device, VkCommandPool& commandPool, VkImageLayout newLayout, uint32_t mipLevels);

		void copyBufferToImage(const std::shared_ptr<my_vulkan::VulkanDevice>& device, VkCommandPool& commandPool, VkBuffer& buffer, uint32_t width, uint32_t height);

		void destroyImage(const VkDevice& device);

		VkImage& getImage() { return image; }
		VkDeviceMemory& getImageMemory() { return imageMemory; }
		VkFormat& getImageFormat() { return format; }
		VkImageLayout& getImageCurrentLayout() { return layout; }
		VkImageView& getImageView() { return imageView; }

	private:
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkFormat format;
		VkImageLayout layout;
	};
}


