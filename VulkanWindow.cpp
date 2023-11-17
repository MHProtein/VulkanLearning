#include "VulkanWindow.h"

#include <vector>


my_vulkan::VulkanWindow::VulkanWindow(uint32_t width, uint32_t height, const std::string& title) : w{width}, h{height}, window_title{title}
{
	framebufferResized = false;
	initWindow();
}

void my_vulkan::VulkanWindow::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(w, h, window_title.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
}

void my_vulkan::framebufferResizedCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}


my_vulkan::VulkanWindow::~VulkanWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}
