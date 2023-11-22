#include "BlinnPhongTexture.h"

#include <iostream>

#include "Camera.h"
#include "VulkanDescriptors.h"
#include "VulkanUniformBuffers.h"
#include "VulkanImage.h"
#include "glm/gtx/io.hpp"

my_vulkan::BlinnPhongTexture::BlinnPhongTexture(const std::string& filePath,
                                                const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool)
	: Texture(filePath, device, commandPool)
{
	ubo = new FragmentUniformBufferObject;
	ubo->ks = { 0.8f, 0.8f, 0.8f };
	ubo->cameraPos = { 0, 0, 0 };
	ubo->lightPos = { 1000,1000, 1000 };
	ubo->lightIntensity = 300.0f;

	uniformBuffer = std::make_shared<VulkanUniformBuffers>(device, VulkanUBOFor::FRAGMENT_SHADER);
	
	uboDescriptor = std::make_shared<VulkanDescriptors>(device, uniformBuffer.get(), 
		VK_NULL_HANDLE, VK_NULL_HANDLE, VulkanDescriptorFor::FRAGMENT_SHADER_UNIFORM_BUFFER);
}

void my_vulkan::BlinnPhongTexture::update(uint32_t currentFrame,Camera* camera, PointLight* light)
{
	//ubo->cameraPos.x= camera->position.z;
	//ubo->cameraPos.z= -camera->position.x;
	//ubo->cameraPos.y= camera->position.y;
	//ubo->lightPos.x = light->transformation.position.z;
	//ubo->lightPos.z = -light->transformation.position.x;
	//ubo->lightPos.y = light->transformation.position.y;
	ubo->lightPos = light->transformation.position;
	ubo->cameraPos = camera->position;
	ubo->lightIntensity = light->intensity;

	//std::cout << ubo->cameraPos << " " << ubo->lightPos << " " << ubo->lightIntensity << " " << std::endl;

	uniformBuffer->updateUniformBuffer(currentFrame, ubo);
}
