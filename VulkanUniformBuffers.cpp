#define GLM_FORCE_RADIANS

#include "VulkanUniformBuffers.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "imgui.h"
#include "Vertex.h"
#include "VulkanUtils.h"

my_vulkan::VulkanUniformBuffers::VulkanUniformBuffers(const std::shared_ptr<VulkanDevice>& device, VulkanUBOFor type) : type(type), count(0)
{
	createUniformBuffers(device, type);
}

void my_vulkan::VulkanUniformBuffers::createUniformBuffers(const std::shared_ptr<VulkanDevice>& device, VulkanUBOFor type)
{
	VkDeviceSize bufferSize;
	switch (type)
	{
	case VulkanUBOFor::MODEL :
		bufferSize = sizeof(UniformBufferObject);
		break;
	case VulkanUBOFor::COMPUTE_SHADER:
		bufferSize = sizeof(ParameterUBO);
		break;
	}


	uniformBuffers.resize(MAX_RENDER_IMAGES);
	uniformBuffersMemory.resize(MAX_RENDER_IMAGES);
	uniformBuffersMapped.resize(MAX_RENDER_IMAGES);

	for (size_t i = 0; i < MAX_RENDER_IMAGES; ++i)
	{
		VulkanUtils::createBuffer(device, uniformBuffers[i], uniformBuffersMemory[i], bufferSize, 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		vkMapMemory(device->getLogicalDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
	//The buffer stays mapped to this pointer for the application's whole lifetime, this technique is called "persistent mapping"
	//Not having to map the buffer every time we need to update it increases performances
}

void my_vulkan::VulkanUniformBuffers::updateUniformBuffer(uint32_t currentImage, VkExtent2D swapChainExtent)
{
	UniformBufferObject ubo{};
	ImVec2 pos2;
	if (ImGui::IsMouseClicked(0) && ImGui::GetMouseClickedCount(0) - count == 1)
	{
		count++;
		lastRecordedMousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
	}
	if (ImGui::IsMouseClicked(2) && ImGui::GetMouseClickedCount(2) - count == 1)
	{
		count++;
		lastRecordedMousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
	}

	if ((ImGui::IsMouseDown(0) && ImGui::IsMouseDragging(0)))
	{
		pos2 = ImGui::GetMousePos();
		float deltaX = 0.010f * (pos2.x - lastRecordedMousePos.x) / 1920.0f;
		float deltaY = 0.010f * (pos2.y - lastRecordedMousePos.y) / 1080.0f;
		Camera::angle1 += deltaX;
		Camera::angle2 += deltaY;
		float a1 = Camera::angle1;
		float a2 = Camera::angle2;
		Camera::t = Camera::radius * cos(a2);
		Camera::camX = Camera::t * cos(a1);
		Camera::camY = Camera::t * sin(a1);
		Camera::camZ = Camera::radius * sin(a2);
	}

	if ((ImGui::IsMouseDown(2) && ImGui::IsMouseDragging(2)))
	{
		pos2 = ImGui::GetMousePos();
		Camera::radius += 0.005f * (pos2.y - lastRecordedMousePos.y) / 1080.0f;
		float a1 = Camera::angle1;
		float a2 = Camera::angle2;
		Camera::t = Camera::radius * cos(a2);
		Camera::camX = Camera::t * cos(a1);
		Camera::camY = Camera::t * sin(a1);
		Camera::camZ = Camera::radius * sin(a2);
	}

	ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	ubo.view = glm::lookAt(glm::vec3(Camera::camX, Camera::camY, Camera::camZ), glm::vec3(0.0F, 0.0F, 2.0f), glm::vec3(0.0, 0.0, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
	ubo.proj[1][1] *= -1;

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void my_vulkan::VulkanUniformBuffers::DestroyVulkanUniformBuffers(const VkDevice& device)
{
	for (size_t i = 0; i < MAX_RENDER_IMAGES; i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}
}

my_vulkan::VulkanUniformBuffers::~VulkanUniformBuffers()
{

}
