#include "VulkanInstance.h"


my_vulkan::VulkanInstance::VulkanInstance(bool enableValidationLayer, const std::vector<const char*>& validationLayers)
{
	createInstance(enableValidationLayer, validationLayers);
}

void my_vulkan::VulkanInstance::createInstance(bool enableValidationLayer, const std::vector<const char*>& validationLayers)
{
	if (enableValidationLayer && !checkValidationLayerAvailability(validationLayers))
		throw std::runtime_error("validation layer enabled, but not available!");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "NONE";

	uint32_t extentionCount;
	const char** extentions = glfwGetRequiredInstanceExtensions(&extentionCount);

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = extentionCount;
	createInfo.ppEnabledExtensionNames = extentions;

	if (enableValidationLayer)
	{
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance!");
}

bool my_vulkan::VulkanInstance::checkValidationLayerAvailability(const std::vector<const char*>& validationLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> layersProperties(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layersProperties.data());
	for (const auto& layer : validationLayers)
	{
		for (auto properties : layersProperties)
		{
			if (strcmp(properties.layerName, layer))
				return true;
		}
	}
	return false;
}

void my_vulkan::VulkanInstance::destroyInstance()
{
	vkDestroyInstance(instance, nullptr);
}

my_vulkan::VulkanInstance::~VulkanInstance()
{
	
}
