#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace my_vulkan
{
	class VulkanContext;
	class VulkanUniformBuffers;
	class Mesh;
	class VulkanDescriptors;
	class Texture;
	class VulkanDevice;
	class Camera;
	class UniformBufferObject;

	class Object
	{
		const std::vector<std::string> texturePaths;
		const std::vector<std::string> modelPaths;

	public:
		Object(const std::string& name, my_vulkan::VulkanContext* context, const std::vector<std::string>& modelPaths,
			const std::vector<std::string>& texturePaths);

		virtual void tick(uint32_t currentImage, Camera* camera);

		void setPosition(glm::vec3 pos);
		void setPosition(float* pos);
		void setRotation(glm::vec3 rot);
		void setScale(glm::vec3 scale);

		void updateTransformationMatrix();

		void destroyObject(VkDevice device);

		std::string name;

		struct 
		{
			glm::vec3 position;
			glm::vec3 rotation;
			glm::vec3 scale;
		} transformation;

		UniformBufferObject* ubo{};
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::vector<std::shared_ptr<Texture>> textures;
		std::shared_ptr<VulkanUniformBuffers> uniformBuffers;
		std::shared_ptr<VulkanDescriptors> uniformBuffersDescriptors;
	};
}



