#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	class VulkanDevice;
	class VulkanImage;
	class VulkanUtils;

	class VulkanDepthResources
	{
	public:
		VulkanDepthResources(const std::shared_ptr<VulkanDevice>& device, VkExtent2D extent, VkCommandPool& commandPool);
		void createDepthBuffer(const std::shared_ptr<VulkanDevice>& device, VkExtent2D extent, VkCommandPool& commandPool);

		static VkFormat findDepthFormat(const std::shared_ptr<VulkanDevice>& device);

		bool hasStencilComponent(VkFormat format) const { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }

		VkImage& getImage();
		VkDeviceMemory& getImageMemory();
		VkFormat& getImageFormat();
		VkImageLayout& getImageCurrentLayout();
		VkImageView& getImageView();

	private:
		std::shared_ptr<VulkanImage> image;
	};

	
}


