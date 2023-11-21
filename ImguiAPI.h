#pragma once
#include <memory>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"
#include "vulkan/vulkan.h"

namespace my_vulkan
{
	class Camera;
	class Object;
	class VulkanContext;
	class ImguiAPI
	{
		
	public:
		ImguiAPI(VulkanContext* context);
		void handleInput(VulkanContext* context, Camera* camera);
		void updateImgui(VkCommandBuffer commandBuffer, const std::vector<std::shared_ptr<Object>>& objects);

	private:
		bool show_demo_window = true;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		ImGuiIO io;
		ImGuiTable table;
	};
}


