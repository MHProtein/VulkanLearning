#pragma once
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	class VulkanUniformBuffers;
	class Model;
	class VulkanDescriptors;
	class Texture;
	class VulkanDevice;

	class Object
	{
		const std::vector<std::string> texturePaths;
		const std::vector<std::string> modelPaths;

	public:
		Object(const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool, const std::vector<std::string>& modelPaths,
			const std::vector<std::string>& texturePaths);

		void destroyObject(VkDevice device);

		std::vector<std::shared_ptr<Model>> models;
		std::vector<std::shared_ptr<Texture>> textures;
		std::shared_ptr<VulkanUniformBuffers> uniformBuffers;
		std::shared_ptr<VulkanDescriptors> uniformBuffersDescriptors;
	};
}



