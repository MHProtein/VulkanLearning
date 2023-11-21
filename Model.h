#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Vertex.h"
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	class VulkanDevice;
	class Texture;

	class Mesh
	{
	public:
		Mesh(const std::string& model_path, const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool);
		void loadModel();

		void createBuffers(const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool);

		std::vector<Vertex> getVertices() const;
		std::vector<uint32_t> getIndices() const { return indices; }

		void destroyModel(const VkDevice& device);

		void Render(const VkCommandBuffer& commandBuffer);


		std::string modelPath;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
	};
}



