#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	class VulkanImage;
	class VulkanDevice;
	class VulkanDescriptors;

	class Texture
	{
	public:
		Texture(const std::string& filePath, const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool);
		void createTextureImage(const std::string& filePath, const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool);

		void createTextureSampler(const std::shared_ptr<VulkanDevice>& device);
		void createDescriptor(const std::shared_ptr<VulkanDevice>& device);
		void generateMipmaps(const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void destroyTexture(VkDevice device);

		std::shared_ptr<VulkanImage>& getTextureImage() { return textureImage; }
		VkSampler& getTextureSampler() { return sampler; }

		bool operator==(const Texture& rhs) const { return name == rhs.name; }
		bool operator==(const std::string& rhs) const { return name == rhs; }

		uint32_t mipLevels;
		std::string name;
		std::shared_ptr<VulkanImage> textureImage;
		std::shared_ptr<VulkanDescriptors> descriptor;
		VkSampler sampler;
	};


}

