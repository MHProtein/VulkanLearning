#include "VulkanDescriptors.h"

#include <stdexcept>

#include "VulkanImage.h"
#include "Texture.h"
#include "VulkanUniformBuffers.h"
#include "VulkanDevice.h"
#include "VulkanUtils.h"
#include "Vertex.h""

my_vulkan::VulkanDescriptors::VulkanDescriptors(const std::shared_ptr<VulkanDevice>& device,
                                                VkBuffer* uniformBuffers, Texture* texture, VulkanDescriptorFor layout_type)
{
	createDescriptorSetLayout(device->getLogicalDevice(), layout_type);
	createDescriptorPool(device->getLogicalDevice(), layout_type);
	createDescriptorSets(device->getLogicalDevice(), uniformBuffers, texture, layout_type);
}

my_vulkan::VulkanDescriptors::VulkanDescriptors(const std::shared_ptr<VulkanDevice>& device,
	VkBuffer* uniformBuffers, VkImageView& imageView, VkSampler& sampler,
	VulkanDescriptorFor layout_type)
{
	createDescriptorSetLayout(device->getLogicalDevice(), layout_type);
	createDescriptorPool(device->getLogicalDevice(), layout_type);
	createDescriptorSets(device->getLogicalDevice(), uniformBuffers, imageView, sampler, layout_type);
}

void my_vulkan::VulkanDescriptors::createDescriptorSetLayout(const VkDevice& device, VulkanDescriptorFor layout_type)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	switch (layout_type)
	{
	
		case VulkanDescriptorFor::UNIFORM_BUFFER:
		{
			VkDescriptorSetLayoutBinding LayoutBinding{};
			LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			LayoutBinding.descriptorCount = 1;
			LayoutBinding.binding = 0;
			LayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			bindings.push_back(LayoutBinding);
			break;
		}
		case VulkanDescriptorFor::COMBINED_IMAGE_SAMPLER:
		{
			VkDescriptorSetLayoutBinding LayoutBinding{};		
			LayoutBinding.binding = 0;
			LayoutBinding.descriptorCount = 1;
			LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			LayoutBinding.pImmutableSamplers = nullptr;
			LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings.push_back(LayoutBinding);
			break;
		}
		case VulkanDescriptorFor::COMPUTE_SHADER:
		{
			bindings.resize(3);
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[0].pImmutableSamplers = nullptr;
			bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

			bindings[1].binding = 1;
			bindings[1].descriptorCount = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[1].pImmutableSamplers = nullptr;
			bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

			bindings[0].binding = 2;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[0].pImmutableSamplers = nullptr;
			bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		}
	}



	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

void my_vulkan::VulkanDescriptors::createDescriptorPool(const VkDevice& device, VulkanDescriptorFor layout_type)
{

	VkDescriptorPoolSize poolSize{};
	switch (layout_type)
	{
		case VulkanDescriptorFor::UNIFORM_BUFFER:
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
		case VulkanDescriptorFor::COMPUTE_SHADER:
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

void my_vulkan::VulkanDescriptors::createDescriptorSets(const VkDevice& device, VkBuffer* uniformBuffers, Texture* texture, VulkanDescriptorFor layout_type)
{
	if(layout_type == VulkanDescriptorFor::COMBINED_IMAGE_SAMPLER)
		createDescriptorSets(device, uniformBuffers, texture->getTextureImage()->getImageView(), texture->getTextureSampler(), layout_type);
	else
		createDescriptorSets(device, uniformBuffers, VK_NULL_HANDLE, VK_NULL_HANDLE, layout_type);
}

void my_vulkan::VulkanDescriptors::createDescriptorSets(const VkDevice& device,
	VkBuffer* uniformBuffers, const VkImageView& imageView, const VkSampler& sampler,
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
		case VulkanDescriptorFor::UNIFORM_BUFFER:
		{
			for (int i = 0; i != MAX_RENDER_IMAGES; ++i)
			{
				VkWriteDescriptorSet writeInfo{};
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof UniformBufferObject;

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
		case VulkanDescriptorFor::COMPUTE_SHADER:
		{
			for (int i = 0; i != MAX_RENDER_IMAGES; ++i)
			{
				VkWriteDescriptorSet writeInfo{};
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers[i];
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
