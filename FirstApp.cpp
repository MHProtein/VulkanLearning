#include <random>

#include "FirstApp.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanRenderer.h"
#include "VulkanUniformBuffers.h"
#include "VulkanDescriptors.h"
#include "Object.h"
#include "VulkanComputePipeline.h"
#include "VulkanUtils.h"
#include "Vertex.h"
#include <imgui.h>
#include <iostream>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <misc/cpp/imgui_stdlib.h>

my_vulkan::FirstApp::FirstApp()
{
	instance = std::make_shared<VulkanInstance>(enableValidationLayer, validationLayers);
	createWindowSurface();
	device = std::make_shared<VulkanDevice>(enableValidationLayer, instance->getInstance(), surface, deviceExtensions, validationLayers);
	swapChain = std::make_shared<VulkanSwapChain>(device->getPhysicalDevice(), device->getLogicalDevice(), surface, wind.window);
	createCommandPool(device->getLogicalDevice(), device->getPhysicalDevice());

	arona = std::make_shared<Object>(device, commandPool, aronaModelPaths, aronaTexturePaths);
	graphicsPipeline = std::make_shared<VulkanGraphicsPipeline>(device, swapChain, commandPool);
	initImgui();
	renderer = std::make_shared<VulkanRenderer>(device, swapChain, commandPool, graphicsPipeline->getRenderPass());
}

void my_vulkan::FirstApp::createSSBO(const std::shared_ptr<VulkanDevice>& device, VkCommandPool& commandPool)
{
	static std::default_random_engine rndEngine((unsigned)time(nullptr));
	static std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

	std::vector<Particle> particles(PARTICLE_COUNT);
	std::vector<ParameterUBO> UBOs(MAX_RENDER_IMAGES);
	for (auto& particle : particles)
	{
		float r = 0.25f * sqrt(rndDist(rndEngine));
		float theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
		float x = r * cos(theta) * HEIGHT / WIDTH;
		float y = r * sin(theta);
		particle.position = glm::vec2(x, y);
		particle.velocity = glm::normalize(glm::vec2(x, y));
		particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
	}

	VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VulkanUtils::createBuffer(device, stagingBuffer, stagingBufferMemory, bufferSize, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, particles.data(), bufferSize);
	vkUnmapMemory(device->getLogicalDevice(), stagingBufferMemory);

	for (int i = 0; i != MAX_RENDER_IMAGES; ++i)
	{
		VulkanUtils::createBuffer(device, shaderStorageBuffers[i], shaderStorageBuffersMemory[i], bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		VulkanUtils::copyBuffer(device->getLogicalDevice(), stagingBuffer, shaderStorageBuffers[i], bufferSize, device->getGraphicsQueue(), commandPool);
	}

	ComputeUniformBuffers = std::make_shared<VulkanUniformBuffers>(device, VulkanUBOFor::COMPUTE_SHADER);
	shaderStorageBuffersDescriptors = std::make_shared<VulkanDescriptors>(device, ComputeUniformBuffers->getUniformBuffers().data(), nullptr, 
		VulkanDescriptorFor::COMPUTE_SHADER);

}

void my_vulkan::FirstApp::run()
{
	while (!glfwWindowShouldClose(wind.window))
	{
		glfwPollEvents();

		renderer->draw(wind, swapChain, graphicsPipeline, device, surface, commandPool, {arona} );
	}
}

void my_vulkan::FirstApp::initImgui()
{
	VkDescriptorPoolSize poolSize[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = std::size(poolSize);
	poolInfo.pPoolSizes = poolSize;

	VkDescriptorPool imguiPool;
	vkCreateDescriptorPool(device->getLogicalDevice(), &poolInfo, nullptr, &imguiPool);

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui_ImplGlfw_InitForVulkan(wind.window, true);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = instance->getInstance();
	initInfo.PhysicalDevice = device->getPhysicalDevice();
	initInfo.Device = device->getLogicalDevice();
	initInfo.Queue = device->getGraphicsQueue();
	initInfo.DescriptorPool = imguiPool;
	initInfo.MinImageCount = MAX_RENDER_IMAGES;
	initInfo.ImageCount = MAX_RENDER_IMAGES;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_8_BIT;
	initInfo.Subpass = 0;

	ImGui_ImplVulkan_Init(&initInfo, graphicsPipeline->getRenderPass());

	ImGui_ImplVulkan_CreateFontsTexture();

	ImGui_ImplVulkan_DestroyFontsTexture();

}

void my_vulkan::FirstApp::createWindowSurface()
{
	if (glfwCreateWindowSurface(instance->getInstance(), wind.window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");
}

void my_vulkan::FirstApp::createCommandPool(const VkDevice& device, const VkPhysicalDevice& physicalDevice)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = VulkanDevice::queryQueueFamilyIndices(physicalDevice).graphicsAndComputeQueue.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create command pool!");
}

my_vulkan::FirstApp::~FirstApp()
{
	arona->destroyObject(device->getLogicalDevice());
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplVulkan_Shutdown();
	ImGui::DestroyContext();
	vkDestroyCommandPool(device->getLogicalDevice(), commandPool, nullptr);
	vkDestroySurfaceKHR(instance->getInstance(), surface, nullptr);
	vkDestroyCommandPool(device->getLogicalDevice(), commandPool, nullptr);
	renderer->destroyRenderer(device->getLogicalDevice());
	graphicsPipeline->destroyGraphicsPipeline(device->getLogicalDevice());
	swapChain->DestroySwapChain(device->getLogicalDevice());
	device->destroyDevice();
	instance->destroyInstance();
}


