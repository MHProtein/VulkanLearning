#include "VulkanDescriptors.h"

#include <stdexcept>

#include "VulkanImage.h"
#include "Texture.h"
#include "VulkanUniformBuffers.h"
#include "VulkanDevice.h"
#include "VulkanUtils.h"
#include "Vertex.h""
#include "VulkanBuffer.h"



my_vulkan::VulkanDescriptors::VulkanDescriptors(const std::shared_ptr<VulkanDevice>& device,
	VulkanUniformBuffers* uniformBuffers, VkImageView imageView, VkSampler sampler,
	VulkanDescriptorFor layout_type)
{
	descriptorSetLayout = VulkanUtils::createDescriptorSetLayout(device->getLogicalDevice(), layout_type);
	createDescriptorPool(device->getLogicalDevice(), layout_type);
	createDescriptorSets(device->getLogicalDevice(), uniformBuffers, imageView, sampler, layout_type);
}

void my_vulkan::VulkanDescriptors::createDescriptorPool(const VkDevice& device, VulkanDescriptorFor layout_type)
{

	VkDescriptorPoolSize poolSize{};
	switch (layout_type)
	{
		case VulkanDescriptorFor::FRAGMENT_SHADER_UNIFORM_BUFFER:
		case VulkanDescriptorFor::VERTEX_SHADER_UNIFORM_BUFFER:
		{
			poolSize.descriptorCount = MAX_RENDER_IMAGES;
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		}
		case VulkanDescriptorFor::COMBINED_IMAGE_SAMPLER:
		{
			poolSize.descriptorCount = MAX_RENDER_IMAGES;
			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		}
		case VulkanDescriptorFor::COMPUTE_SHADER_UNIFORM_BUFFER:
		{
			poolSize.descriptorCount = MAX_RENDER_IMAGES * 2;
			poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;
		}
	}

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.maxSets = MAX_RENDER_IMAGES;
	createInfo.poolSizeCount = 1;
	createInfo.pPoolSizes = &poolSize;

	if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool!");
}

void my_vulkan::VulkanDescriptors::createDescriptorSets(const VkDevice& device,
	VulkanUniformBuffers* uniformBuffers, const VkImageView& imageView, const VkSampler& sampler,
	VulkanDescriptorFor layout_type)
{
	std::vector descriptorSetLayouts(MAX_RENDER_IMAGES, descriptorSetLayout);
	descriptorSets.resize(MAX_RENDER_IMAGES);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = MAX_RENDER_IMAGES;
	allocInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate descriptor sets!");

	switch (layout_type)
	{
		case VulkanDescriptorFor::VERTEX_SHADER_UNIFORM_BUFFER:
		{
			for (int i = 0; i != MAX_RENDER_IMAGES; ++i)
			{
				VkWriteDescriptorSet writeInfo{};
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers->getUniformBuffers()[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof VertexUniformBufferObject;

				writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeInfo.descriptorCount = 1;
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeInfo.dstSet = descriptorSets[i];
				writeInfo.dstBinding = 0;
				writeInfo.dstArrayElement = 0;
				writeInfo.pBufferInfo = &bufferInfo;
				vkUpdateDescriptorSets(device, 1, &writeInfo, 0, nullptr);
			}
			break;
		}
		case VulkanDescriptorFor::FRAGMENT_SHADER_UNIFORM_BUFFER:
		{
			for (int i = 0; i != MAX_RENDER_IMAGES; ++i)
			{
				VkWriteDescriptorSet writeInfo{};
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers->getUniformBuffers()[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof FragmentUniformBufferObject;

				writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeInfo.descriptorCount = 1;
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeInfo.dstSet = descriptorSets[i];
				writeInfo.dstBinding = 0;
				writeInfo.dstArrayElement = 0;
				writeInfo.pBufferInfo = &bufferInfo;
				vkUpdateDescriptorSets(device, 1, &writeInfo, 0, nullptr);
			}
			break;
		}
		case VulkanDescriptorFor::COMBINED_IMAGE_SAMPLER:
		{
			for (int i = 0; i != MAX_RENDER_IMAGES; ++i)
			{
				VkWriteDescriptorSet writeInfo{};
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = imageView;
				imageInfo.sampler = sampler;

				writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeInfo.descriptorCount = 1;
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeInfo.dstSet = descriptorSets[i];
				writeInfo.dstBinding = 0;
				writeInfo.dstArrayElement = 0;
				writeInfo.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(device, 1, &writeInfo, 0, nullptr);
			}
			break;
		}
		case VulkanDescriptorFor::COMPUTE_SHADER_UNIFORM_BUFFER:
		{
			for (int i = 0; i != MAX_RENDER_IMAGES; ++i)
			{
				VkWriteDescriptorSet writeInfo{};
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers->getUniformBuffers()[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(Particle) * PARTICLE_COUNT;

				writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeInfo.descriptorCount = 1;
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeInfo.dstSet = descriptorSets[i];
				writeInfo.dstBinding = 0;
				writeInfo.dstArrayElement = 0;
				writeInfo.pBufferInfo = &bufferInfo;
				vkUpdateDescriptorSets(device, 1, &writeInfo, 0, nullptr);
			}
			break;
		}
	}
}

void my_vulkan::VulkanDescriptors::DestroyVulkanDescriptor(const VkDevice& device)
{
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}
