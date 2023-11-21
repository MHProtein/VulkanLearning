#include <random>
#include "VulkanContext.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanRenderer.h"
#include "Object.h"
#include "VulkanUtils.h"
#include "Vertex.h"
#include "Camera.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "Arona.h"
#include "imgui_internal.h"
#include "VulkanUtils.h"

my_vulkan::VulkanContext::VulkanContext() : startTime(clock.now())
{
	
	instance = std::make_shared<VulkanInstance>(enableValidationLayer, validationLayers);

	createWindowSurface();

	device = std::make_shared<VulkanDevice>(enableValidationLayer, instance->getInstance(), surface, deviceExtensions, validationLayers);

	swapChain = std::make_shared<VulkanSwapChain>(device->getPhysicalDevice(), device->getLogicalDevice(), surface, wind.window);

	createCommandPool(device->getLogicalDevice(), device->getPhysicalDevice());

	graphicsPipeline = std::make_shared<VulkanGraphicsPipeline>(device, swapChain, commandPool);
}

void my_vulkan::VulkanContext::createWindowSurface()
{
	if (glfwCreateWindowSurface(instance->getInstance(), wind.window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");
}

void my_vulkan::VulkanContext::createCommandPool(const VkDevice& device, const VkPhysicalDevice& physicalDevice)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = VulkanDevice::queryQueueFamilyIndices(physicalDevice).graphicsAndComputeQueue.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create command pool!");
}

my_vulkan::VulkanContext::~VulkanContext()
{
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplVulkan_Shutdown();
	ImGui::DestroyContext();
	vkDestroyCommandPool(device->getLogicalDevice(), commandPool, nullptr);
	vkDestroySurfaceKHR(instance->getInstance(), surface, nullptr);
	vkDestroyCommandPool(device->getLogicalDevice(), commandPool, nullptr);
	graphicsPipeline->destroyGraphicsPipeline(device->getLogicalDevice());
	swapChain->DestroySwapChain(device->getLogicalDevice());
	device->destroyDevice();
	instance->destroyInstance();
}


