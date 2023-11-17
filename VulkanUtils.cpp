#include "VulkanUtils.h"
#include "VulkanDevice.h"
#include "Vertex.h"

uint32_t my_vulkan::VulkanUtils::findMemoryType(const VkPhysicalDevice& physicalDevice,
	VkMemoryPropertyFlags requiredProperties, uint32_t filters)
{
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((filters & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}
}

void my_vulkan::VulkanUtils::createBuffer(const std::shared_ptr<VulkanDevice>& device, VkBuffer& buffer,
	VkDeviceMemory& memory, VkDeviceSize size, VkMemoryPropertyFlags requiredProperties, VkBufferUsageFlags usage)
{
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.usage = usage;

	if (vkCreateBuffer(device->getLogicalDevice(), &createInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer!");

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(device->getLogicalDevice(), buffer, &requirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = requirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(device->getPhysicalDevice(), requiredProperties, requirements.memoryTypeBits);

	if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate buffer memory!");

	vkBindBufferMemory(device->getLogicalDevice(), buffer, memory, 0);
}

void my_vulkan::VulkanUtils::endSingleTimeCommands(const VkDevice& device, VkCommandBuffer& commandBuffer, VkCommandPool& commandPool, const VkQueue& queueToSubmit)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	if(queueToSubmit != VK_NULL_HANDLE)
	{
		vkQueueSubmit(queueToSubmit, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queueToSubmit);
	}

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkFormat my_vulkan::VulkanUtils::findSupportFormat(const std::shared_ptr<VulkanDevice>& device,
	const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (const VkFormat& format : candidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			return format;
	}
	throw std::runtime_error("failed to find supported format!");
}

void my_vulkan::VulkanUtils::createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory,
	const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VulkanUtils::createBuffer(device, stagingBuffer, stagingBufferMemory, bufferSize,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device->getLogicalDevice(), stagingBufferMemory);

	VulkanUtils::createBuffer(device, vertexBuffer, vertexBufferMemory, bufferSize,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

	VulkanUtils::copyBuffer(device->getLogicalDevice(), stagingBuffer, vertexBuffer, size, device->getGraphicsQueue(), commandPool);

	vkDestroyBuffer(device->getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device->getLogicalDevice(), stagingBufferMemory, nullptr);
}

void my_vulkan::VulkanUtils::createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory,
	const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool)
{
	VkDeviceSize size = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VulkanUtils::createBuffer(device, stagingBuffer, stagingBufferMemory, size,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBufferMemory, 0, size, 0, &data);
	memcpy(data, indices.data(), size);
	vkUnmapMemory(device->getLogicalDevice(), stagingBufferMemory);

	VulkanUtils::createBuffer(device, indexBuffer, indexBufferMemory, size,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	VulkanUtils::copyBuffer(device->getLogicalDevice(), stagingBuffer, indexBuffer, size, device->getGraphicsQueue(), commandPool);

	vkDestroyBuffer(device->getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device->getLogicalDevice(), stagingBufferMemory, nullptr);
}

VkDescriptorSetLayout my_vulkan::VulkanUtils::createDescriptorSetLayout(const VkDevice& device,
	VulkanDescriptorFor layout_type)
{
	VkDescriptorSetLayoutBinding LayoutBinding{};
	switch (layout_type)
	{
	case VulkanDescriptorFor::UNIFORM_BUFFER:
	{
		LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		LayoutBinding.descriptorCount = 1;
		LayoutBinding.binding = 0;
		LayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	}
	case VulkanDescriptorFor::COMBINED_IMAGE_SAMPLER:
	{
		LayoutBinding.binding = 0;
		LayoutBinding.descriptorCount = 1;
		LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		LayoutBinding.pImmutableSamplers = nullptr;
		LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	}
	default:
		break;
	}

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { LayoutBinding };

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout;

	if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
	return descriptorSetLayout;
}

VkShaderModule my_vulkan::VulkanUtils::createShaderModule(const std::vector<char>& shader, const VkDevice& device)
{
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shader.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("failed to create shade module!");
	return shaderModule;
}


void my_vulkan::VulkanUtils::copyBuffer(const VkDevice& device, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                        VkDeviceSize size, const VkQueue& graphicsQueue, VkCommandPool& commandPool)
{
	VkBufferCopy region;
	region.size = size;
	region.dstOffset = 0;
	region.srcOffset = 0;

	VkCommandBuffer commandBuffer = beginSingleTimeCommand(device, commandPool);

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);

	endSingleTimeCommands(device, commandBuffer, commandPool, graphicsQueue);
}


std::vector<char> my_vulkan::VulkanUtils::readFile(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		throw std::runtime_error("cannot open file : " + filePath);
	size_t fileSize = file.tellg();
	std::vector<char> ret(file.tellg());
	file.seekg(0);
	file.read(ret.data(), fileSize);
	return ret;
}

VkCommandBuffer my_vulkan::VulkanUtils::beginSingleTimeCommand(const VkDevice& device,
	VkCommandPool& commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffer memory!");

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pInheritanceInfo = nullptr;
	beginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}
