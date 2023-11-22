#pragma once
#include "PointLight.h"
#include "Texture.h"
#include "VulkanUtils.h"

namespace my_vulkan
{
	class Camera;
}

namespace my_vulkan
{
	class VulkanUniformBuffers;

	class BlinnPhongTexture : public Texture
	{
	public:
		BlinnPhongTexture(const std::string& filePath, const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool);

		void update(uint32_t currentFrame, Camera* camera, PointLight* light);

		std::shared_ptr<VulkanUniformBuffers> uniformBuffer;
		std::shared_ptr<VulkanDescriptors> uboDescriptor;

	private:
		FragmentUniformBufferObject* ubo;
	};
}


