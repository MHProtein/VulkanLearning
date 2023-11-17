#pragma once
#include <string>
#include <GLFW/glfw3.h>
namespace my_vulkan
{

	static void framebufferResizedCallback(GLFWwindow* window, int width, int height);

	class VulkanWindow
	{
	public:
		VulkanWindow(uint32_t width, uint32_t height, const std::string& title);

		void initWindow();

		GLFWwindow* getWindow() { return window; }

		~VulkanWindow();

		bool framebufferResized;
		uint32_t w;
		uint32_t h;
		std::string window_title;
		GLFWwindow* window;
		
	};
}


