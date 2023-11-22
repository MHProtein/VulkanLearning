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
#include "vulkan/vulkan.h"
#include "BlinnPhongTexture.h"

my_vulkan::Object::Object(const std::string& name, my_vulkan::VulkanContext* context, const std::vector<std::string>& modelPaths,
                          const std::vector<std::string>& texturePaths) : modelPaths(modelPaths), texturePaths(texturePaths), name(name)
{
	textures.resize(texturePaths.size());
	meshes.resize(modelPaths.size());

	for(int i = 0; i != texturePaths.size(); ++i)
	{
		textures[i] = std::make_shared<BlinnPhongTexture>(texturePaths[i], context->device, context->commandPool);
		meshes[i] = std::make_shared<Mesh>(modelPaths[i], context->device, context->commandPool);
	}

	transformation.position = { 0, 0, 0 };
	transformation.rotation = { 0, 0, 0 };
	transformation.scale = { 1, 1, 1 };
	ubo = new VertexUniformBufferObject;
	ubo->model = glm::mat4(1.0f);
	updateTransformationMatrix();
	uniformBuffers = std::make_shared<VulkanUniformBuffers>(context->device, VulkanUBOFor::VERTEX_SHADER);
	uniformBuffersDescriptors = std::make_shared<VulkanDescriptors>(context->device, uniformBuffers.get(),
		VK_NULL_HANDLE, VK_NULL_HANDLE, VulkanDescriptorFor::VERTEX_SHADER_UNIFORM_BUFFER);
	moveSpeed = 1.0f;
	rotateSpeed = 2.0f;

}

void my_vulkan::Object::tick(uint32_t currentImage, Camera* camera, PointLight* light)
{
	ubo->view = camera->matrices.view;
	ubo->proj = camera->matrices.perspective;
	ubo->proj[1][1] *= -1;

	uniformBuffers->updateUniformBuffer(currentImage, ubo);

	for (const auto & texture : textures)
	{
		texture->update(currentImage, camera, light);
	}

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

void my_vulkan::Object::Render(uint32_t currentFrame, VkCommandBuffer commandBuffer, VkPipelineLayout layout)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
		&uniformBuffersDescriptors->getDescriptorSets().at(currentFrame), 0, nullptr);
	for (size_t i = 0; i != textures.size(); ++i)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1,
			&textures[i]->sampleDescriptor->getDescriptorSets().at(currentFrame), 0, nullptr);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1,
			&textures[i]->uboDescriptor->getDescriptorSets().at(currentFrame), 0, nullptr);
		meshes[i]->Render(commandBuffer);
	}
}

void my_vulkan::Object::updateTransformationMatrix()
{
	ubo->model = glm::mat4(1);
	ubo->model = glm::translate(ubo->model, transformation.position * moveSpeed);
	ubo->model = glm::rotate(ubo->model, glm::radians(transformation.rotation.y) * rotateSpeed, glm::vec3(0, 1, 0));
	ubo->model = glm::rotate(ubo->model, glm::radians(transformation.rotation.z) * rotateSpeed, glm::vec3(0, 0, 1));
	ubo->model = glm::rotate(ubo->model, glm::radians(transformation.rotation.x) * rotateSpeed, glm::vec3(1, 0, 0));

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
