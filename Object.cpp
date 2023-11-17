#include "Object.h"

#include <iostream>
#include <ostream>
#include "VulkanUtils.h"
#include "Object.h"
#include "VulkanDescriptors.h"
#include "VulkanUniformBuffers.h"
#include "Texture.h"
#include "Model.h"

my_vulkan::Object::Object(const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool, const std::vector<std::string>& modelPaths,
	const std::vector<std::string>& texturePaths) : modelPaths(modelPaths), texturePaths(texturePaths)
{
	textures.resize(texturePaths.size());
	models.resize(modelPaths.size());

	for(int i = 0; i != texturePaths.size(); ++i)
	{
		textures[i] = std::make_shared<Texture>(texturePaths[i], device, commandPool);
		models[i] = std::make_shared<Model>(modelPaths[i], device, commandPool);
	}
	uniformBuffers = std::make_shared<VulkanUniformBuffers>(device, VulkanUBOFor::MODEL);
	uniformBuffersDescriptors = std::make_shared<VulkanDescriptors>(device, uniformBuffers->getUniformBuffers().data(), nullptr, VulkanDescriptorFor::UNIFORM_BUFFER);
}

void my_vulkan::Object::destroyObject(VkDevice device)
{
	for (int i = 0; i != texturePaths.size(); ++i)
	{
		textures[i]->destroyTexture(device);
		models[i]->destroyModel(device);
	}
	uniformBuffers->DestroyVulkanUniformBuffers(device);
	uniformBuffersDescriptors->DestroyVulkanDescriptor(device);
}
