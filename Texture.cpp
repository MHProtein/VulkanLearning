#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <stdexcept>
#include "VulkanUtils.h"
#include <stb_image.h>
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "Texture.h"
#include "VulkanDescriptors.h"
#include "Vertex.h"

my_vulkan::Texture::Texture(const std::string& filePath, const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool)
{
	name.insert(0, filePath.substr(filePath.find_last_of('/') + 1, filePath.find_last_of('.') - filePath.find_last_of('/') - 1));
	createTextureImage(filePath, device, commandPool);
	createTextureSampler(device);
	createDescriptor(device);
}

void my_vulkan::Texture::createTextureImage(const std::string& filePath, const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool)
{
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha; //4 bytes per pixel

	auto mipmapLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(texHeight, texWidth))));
	mipLevels = mipmapLevel;
	if (!pixels)
		throw std::runtime_error("failed to load texture image!"); 

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VulkanUtils::createBuffer(device, stagingBuffer, stagingBufferMemory, imageSize, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, imageSize);
	vkUnmapMemory(device->getLogicalDevice(), stagingBufferMemory);

	stbi_image_free(pixels);

	textureImage = std::make_shared<VulkanImage>(device, texWidth, texHeight, 1, mipmapLevel, 1, 
		VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, 
		VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	textureImage->transitionImageLayout(device, commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipmapLevel);
	textureImage->copyBufferToImage(device, commandPool, stagingBuffer, texWidth, texHeight);

	generateMipmaps(device, commandPool, texWidth, texHeight, mipmapLevel);

	vkDestroyBuffer(device->getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device->getLogicalDevice(), stagingBufferMemory, nullptr);
}

void my_vulkan::Texture::generateMipmaps(const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), textureImage->getImageFormat(), &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = VulkanUtils::beginSingleTimeCommand(device->getLogicalDevice(), commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = textureImage->getImage();
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for(uint32_t i = 1; i < mipLevels; ++i)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0,0,0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			textureImage->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			textureImage->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	VulkanUtils::endSingleTimeCommands(device->getLogicalDevice(), commandBuffer, commandPool, device->getGraphicsQueue());
}

void my_vulkan::Texture::createTextureSampler(const std::shared_ptr<VulkanDevice>& device)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR; //How to interpolate texels when magnified
	samplerInfo.minFilter = VK_FILTER_LINEAR; //or minified
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device->getPhysicalDevice(), &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; //limits the amount of texel samples that can be used to calculate the final color

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //specifies which color is returned when sampling beyond the image with clamp to border addressing mode
	samplerInfo.unnormalizedCoordinates = VK_FALSE; //if true : [0, width] * [0, height], false : [0, 1] ^ 2
	samplerInfo.compareEnable = VK_FALSE; //if true: the texels first will be compared to a value, and the result of the comparison is used in filtering operations.
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = mipLevels;

	if (vkCreateSampler(device->getLogicalDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture sampler!");
}

void my_vulkan::Texture::createDescriptor(const std::shared_ptr<VulkanDevice>& device)
{
	sampleDescriptor = std::make_shared<VulkanDescriptors>(device, nullptr, textureImage->getImageView(), sampler, VulkanDescriptorFor::COMBINED_IMAGE_SAMPLER);
}

void my_vulkan::Texture::destroyTexture(VkDevice device)
{
	sampleDescriptor->DestroyVulkanDescriptor(device);
	vkDestroySampler(device, sampler, nullptr);
	textureImage->destroyImage(device);
}