#include "Object.h"
#define GLM_FORCE_RADIANCE
#include <iostream>
#include <ostream>
#include "VulkanUtils.h"
#include "VulkanDescriptors.h"
#include "VulkanUniformBuffers.h"
#include "Texture.h"
#include "Model.h"
#include "VulkanContext.h"
#include "glm/gtx/io.hpp"
#include "Camera.h"

my_vulkan::Object::Object(const std::string& name, my_vulkan::VulkanContext* context, const std::vector<std::string>& modelPaths,
                          const std::vector<std::string>& texturePaths) : modelPaths(modelPaths), texturePaths(texturePaths), name(name)
{
	textures.resize(texturePaths.size());
	meshes.resize(modelPaths.size());

	for(int i = 0; i != texturePaths.size(); ++i)
	{
		textures[i] = std::make_shared<Texture>(texturePaths[i], context->device, context->commandPool);
		meshes[i] = std::make_shared<Mesh>(modelPaths[i], context->device, context->commandPool);
	}

	transformation.position = { 0, 0, 0 };
	transformation.rotation = { 0, 0, 0 };
	transformation.scale = { 1, 1, 1 };
	ubo = new UniformBufferObject;
	ubo->model = glm::mat4(1.0f);
	updateTransformationMatrix();
	uniformBuffers = std::make_shared<VulkanUniformBuffers>(context->device, VulkanUBOFor::MODEL);
	uniformBuffersDescriptors = std::make_shared<VulkanDescriptors>(context->device, uniformBuffers->getUniformBuffers().data(),
		nullptr, VulkanDescriptorFor::UNIFORM_BUFFER);
}

void my_vulkan::Object::tick(uint32_t currentImage, Camera* camera)
{
	ubo->view = camera->matrices.view;
	ubo->proj = camera->matrices.perspective;
	ubo->proj[1][1] *= -1;

	uniformBuffers->updateUniformBuffer(currentImage, ubo);
}

void my_vulkan::Object::setPosition(glm::vec3 pos)
{
	transformation.position = pos;
	updateTransformationMatrix();
}

void my_vulkan::Object::setPosition(float* pos)
{
	transformation.position.x = pos[0] * 0.0001f;
	transformation.position.y = pos[1] * 0.0001f;
	transformation.position.z = pos[2] * 0.0001f;
	updateTransformationMatrix();
}

void my_vulkan::Object::setRotation(glm::vec3 rot)
{
	transformation.rotation = rot;
	updateTransformationMatrix();
}

void my_vulkan::Object::setScale(glm::vec3 scale)
{
	transformation.scale = scale;
	updateTransformationMatrix();
}

void my_vulkan::Object::updateTransformationMatrix()
{
	ubo->model = glm::mat4(1);
	ubo->model = glm::rotate(ubo->model, glm::radians(transformation.rotation.x), glm::vec3(1, 0, 0));
	ubo->model = glm::rotate(ubo->model, glm::radians(transformation.rotation.y), glm::vec3(0, 1, 0));
	ubo->model = glm::rotate(ubo->model, glm::radians(transformation.rotation.z), glm::vec3(0, 0, 1));
	ubo->model = glm::translate(ubo->model, transformation.position);
	ubo->model = glm::scale(ubo->model, transformation.scale);
}

void my_vulkan::Object::destroyObject(VkDevice device)
{
	delete ubo;
	for (int i = 0; i != texturePaths.size(); ++i)
	{
		textures[i]->destroyTexture(device);
		meshes[i]->destroyModel(device);
	}
	uniformBuffers->DestroyVulkanUniformBuffers(device);
	uniformBuffersDescriptors->DestroyVulkanDescriptor(device);
}
