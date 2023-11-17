#pragma once

#include <array>
#include <fstream>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>
#include <cmath>
#include <iostream>

namespace my_vulkan
{
	struct Vertex;
	class VulkanDevice;
	const uint32_t MAX_RENDER_IMAGES = 2;
	const uint32_t MODEL_COUNT = 6;
	const uint32_t PARTICLE_COUNT = 1000;

	struct Camera
	{
	
		inline static float initCamX = 2.0f;
		inline static float initCamY = 2.0f;
		inline static float initCamZ = 1.0f;
		inline static float camX = initCamX;
		inline static float camY = initCamY;
		inline static float camZ = initCamZ;
		inline static float radius = distance(glm::vec3(initCamX, initCamY, initCamZ), glm::vec3(0.0f, 0.0f, 0.0f));
		inline static float angle1 = acos(glm::dot(normalize(glm::vec3(initCamX, initCamY, 0.0f)),
			normalize(glm::vec3(1.0f, 0.0f, 0.0f))));
		inline static float angle2 = acos((glm::dot(normalize(glm::vec3(initCamX, initCamY, 0.0f)), 
			normalize(glm::vec3(initCamX, initCamY, initCamZ)))));
		inline static float t = radius * cos(angle2);
	};

	enum class VulkanDescriptorFor { UNIFORM_BUFFER, COMBINED_IMAGE_SAMPLER, COMPUTE_SHADER };
	enum class VulkanUBOFor { MODEL, COMPUTE_SHADER };

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsAndComputeQueue;
		std::optional<uint32_t>	presentQueue;
		
		bool isComplete() { return graphicsAndComputeQueue.has_value() && presentQueue.has_value(); }
	};

	struct SwapChainCreateDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct ParameterUBO
	{
		float Delta = 1.0f;
	};

	struct UniformBufferObject
	{
		glm::vec2 foo;  //8
		alignas(16) glm::mat4 model; //offset 8 //offset of each element must rounded up to a multiple of 16
		glm::mat4 view; //offset 8 + 64
		glm::mat4 proj; //offset 72 + 64
	};

	struct Particle {
		glm::vec2 position;
		glm::vec2 velocity;
		glm::vec4 color;

	};

	class VulkanUtils
	{
	public:
		static uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, VkMemoryPropertyFlags requiredProperties, uint32_t filters);

		static void createBuffer(const std::shared_ptr<VulkanDevice>& device, VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size,
		                         VkMemoryPropertyFlags requiredProperties, VkBufferUsageFlags usage);

		static void copyBuffer(const VkDevice& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, const VkQueue& graphicsQueue, VkCommandPool& commandPool);

		static std::vector<char> readFile(const std::string& filePath);

		static VkCommandBuffer beginSingleTimeCommand(const VkDevice& device, VkCommandPool& commandPool);

		static void endSingleTimeCommands(const VkDevice& device, VkCommandBuffer& commandBuffer, VkCommandPool& commandPool, const VkQueue& queueToSubmit);

		static VkFormat findSupportFormat(const std::shared_ptr<VulkanDevice>& device,
			const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		static bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }

		static void createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory,
			const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool);
		static void createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory,
			const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool);

		static VkDescriptorSetLayout createDescriptorSetLayout(const VkDevice& device, VulkanDescriptorFor layout_type);

		static VkShaderModule createShaderModule(const std::vector<char>& shader, const VkDevice& device);

	};


}
