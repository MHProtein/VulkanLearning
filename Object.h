#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	class PointLight;
	class BlinnPhongTexture;
}

namespace my_vulkan
{
	class VulkanContext;
	class VulkanUniformBuffers;
	class Mesh;
	class VulkanDescriptors;
	class Texture;
	class VulkanDevice;
	class Camera;
	class VertexUniformBufferObject;

	class Object
	{
		const std::vector<std::string> texturePaths;
		const std::vector<std::string> modelPaths;

	public:
		Object(const std::string& name, my_vulkan::VulkanContext* context, const std::vector<std::string>& modelPaths,
			const std::vector<std::string>& texturePaths);

		virtual void tick(uint32_t currentImage, Camera* camera, PointLight* light);

		void setPosition(glm::vec3 pos);
		void setPosition(float* pos);
		void setRotation(glm::vec3 rot);
		void setScale(glm::vec3 scale);
		void Render(uint32_t currentFrame, VkCommandBuffer commandBuffer, VkPipelineLayout layout);
		void updateTransformationMatrix();

		void destroyObject(VkDevice device);

		std::string name;
		float moveSpeed;
		float rotateSpeed;
		struct 
		{
			glm::vec3 position;
			glm::vec3 rotation;
			glm::vec3 scale;
		} transformation;

		VertexUniformBufferObject* ubo{};
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::vector<std::shared_ptr<BlinnPhongTexture>> textures;
		std::shared_ptr<VulkanUniformBuffers> uniformBuffers;
		std::shared_ptr<VulkanDescriptors> uniformBuffersDescriptors;
	};
}



