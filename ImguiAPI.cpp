#include "ImguiAPI.h"

#include <iostream>

#include "VulkanUtils.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "VulkanContext.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanRenderer.h"
#include "Object.h"
#include "Vertex.h"
#include "Camera.h"
#include "imgui_internal.h"
#include "glm/gtc/type_ptr.hpp"

my_vulkan::ImguiAPI::ImguiAPI(VulkanContext* context)
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
	vkCreateDescriptorPool(context->device->getLogicalDevice(), &poolInfo, nullptr, &imguiPool);

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui_ImplGlfw_InitForVulkan(context->wind.window, true);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = context->instance->getInstance();
	initInfo.PhysicalDevice = context->device->getPhysicalDevice();
	initInfo.Device = context->device->getLogicalDevice();
	initInfo.Queue = context->device->getGraphicsQueue();
	initInfo.DescriptorPool = imguiPool;
	initInfo.MinImageCount = MAX_RENDER_IMAGES;
	initInfo.ImageCount = MAX_RENDER_IMAGES;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_8_BIT;
	initInfo.Subpass = 0;

	ImGui_ImplVulkan_Init(&initInfo, context->graphicsPipeline->getRenderPass());

	ImGui_ImplVulkan_CreateFontsTexture();

	ImGui_ImplVulkan_DestroyFontsTexture();

}

void my_vulkan::ImguiAPI::handleInput(VulkanContext* context, Camera* camera)
{
	if (io.MouseDown[1])
	{
		auto mouseDelta = io.MouseDelta;
		//PITCH              //YAW                      //ROLL
		camera->rotate({ -mouseDelta.y * 0.5f,-mouseDelta.x * 0.5f , 0 });
	}
	if (ImGui::IsKeyDown(ImGuiKey_E))
		camera->rotate({ 0, 0, 0.1 });
	if (ImGui::IsKeyDown(ImGuiKey_Q))
		camera->rotate({ 0, 0, -0.1 });
	if (ImGui::IsKeyDown(ImGuiKey_W))
		camera->moveForward();
	if (ImGui::IsKeyDown(ImGuiKey_S))
		camera->moveBack();
	if (ImGui::IsKeyDown(ImGuiKey_A))
		camera->moveLeft();
	if (ImGui::IsKeyDown(ImGuiKey_D))
		camera->moveRight();
	if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
		camera->moveUp();
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
		camera->moveDown();
}

void my_vulkan::ImguiAPI::updateImgui(VkCommandBuffer commandBuffer, const std::vector<std::shared_ptr<Object>>& objects)
{
	io = ImGui::GetIO();
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow(&show_demo_window);
	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	//ImGui::SliderFloat("float", &objects[0]->transformation.scale.x, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	//objects[0]->updateTransformationMatrix();
	//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
	//	counter++;
	//ImGui::SameLine();
	//ImGui::Text("counter = %d", counter);

	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

	for (auto & object : objects)
	{
		ImGui::BeginListBox(object->name.c_str());
		if (ImGui::DragFloat3("Position", value_ptr(object->transformation.position), 0.1f) ||
			ImGui::DragFloat3("Rotation", value_ptr(object->transformation.rotation), 0.1f) ||
			ImGui::DragFloat3("Scale", value_ptr(object->transformation.scale), 0.1f, 0.0000f, std::numeric_limits<float>::max()))
			object->updateTransformationMatrix();
		ImGui::EndListBox();
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
