#define TINYOBJLOADER_IMPLEMENTATION
#include "Model.h"

#include <tiny_obj_loader.h>
#include <unordered_map>
#include "VulkanUtils.h"
#include "Texture.h"
#include "Vertex.h"

my_vulkan::Mesh::Mesh(const std::string& model_path, const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool) : modelPath(model_path)
{
	loadModel();
	createBuffers(device, commandPool);
}

void my_vulkan::Mesh::loadModel()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.data()))
		throw std::runtime_error(warn + err);

	std::unordered_map<Vertex, int32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for(const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			vertex.color = { 1.0f, 1.0f, 1.0f };

			if(uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void my_vulkan::Mesh::createBuffers(const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool)
{
	VulkanUtils::createVertexBuffer(vertices, vertexBuffer, vertexBufferMemory, device, commandPool);
	VulkanUtils::createIndexBuffer(indices, indexBuffer, indexBufferMemory, device, commandPool);
}

std::vector<my_vulkan::Vertex> my_vulkan::Mesh::getVertices() const
{
	return vertices; 
}

void my_vulkan::Mesh::destroyModel(const VkDevice& device)
{
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
}

void my_vulkan::Mesh::Render(const VkCommandBuffer& commandBuffer)
{
	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}
