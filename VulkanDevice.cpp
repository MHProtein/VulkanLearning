#include "VulkanDevice.h"

#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

#include "VulkanUtils.h"

my_vulkan::VulkanDevice::VulkanDevice(bool enableValidationLayer, const VkInstance& instance, VkSurfaceKHR surface, 
                                      const std::vector<const char*>& deviceExtensions, const std::vector<const char*>& validationLayers)
{
	pickPhysicalDevice(instance, surface, deviceExtensions);
	createLogicalDevice(enableValidationLayer, validationLayers, deviceExtensions);
}

VkSampleCountFlagBits my_vulkan::VulkanDevice::getMaxUsableSampleCount()
{
	return VK_SAMPLE_COUNT_8_BIT;
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_64_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

void my_vulkan::VulkanDevice::pickPhysicalDevice(const VkInstance& instance, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions)
{
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
	for (auto device : physicalDevices)
	{
		if (physicalDeviceSuitable(device, surface, deviceExtensions))
		{
			physicalDevice = device;
			msaaSamples = getMaxUsableSampleCount();
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("failed to create physical device!");
}

bool my_vulkan::VulkanDevice::physicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions)
{
	auto indices = queryQueueFamilyIndices(device);
	auto details = querySwapChainCreateDetails(device, surface);
	bool detailAdequate = !details.presentModes.empty() && !details.formats.empty();

	VkPhysicalDeviceFeatures supprotedFreatures{};
	vkGetPhysicalDeviceFeatures(device, &supprotedFreatures);

	return physicalDeviceExtensionsCheck(device, deviceExtensions) && indices.isComplete() && detailAdequate && supprotedFreatures.samplerAnisotropy;
}

bool my_vulkan::VulkanDevice::physicalDeviceExtensionsCheck(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions)
{
	uint32_t propertyCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, nullptr);
	std::vector<VkExtensionProperties> extensionProperties(propertyCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, extensionProperties.data());

	std::set extentionTemp(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : extentionTemp)
	{
		for (auto prop : extensionProperties)
		{
			if (strcmp(extension, prop.extensionName))
				extentionTemp.erase(extension);
		}
	}
	return extentionTemp.empty();
}

my_vulkan::QueueFamilyIndices my_vulkan::VulkanDevice::queryQueueFamilyIndices(VkPhysicalDevice device)
{
	QueueFamilyIndices indices{};
	uint32_t QueueFamilyPropertyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyPropertyCount, nullptr);
	std::vector<VkQueueFamilyProperties> QueueFamiliesProperties(QueueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyPropertyCount, QueueFamiliesProperties.data());
	int i = 0;
	for (auto prop : QueueFamiliesProperties)
	{
		if ((prop.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) && (prop.queueFlags & VK_QUEUE_COMPUTE_BIT))
			indices.graphicsAndComputeQueue = i;
		if (vkGetPhysicalDeviceWin32PresentationSupportKHR(device, i) == VK_TRUE)
			indices.presentQueue = i;
		if (indices.isComplete())
			break;
	}
	return indices;
}

my_vulkan::SwapChainCreateDetails my_vulkan::VulkanDevice::querySwapChainCreateDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainCreateDetails details{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, nullptr);
	if (surfaceFormatCount)
	{
		details.formats.resize(surfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, details.formats.data());
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (surfaceFormatCount)
	{
		details.presentModes.resize(surfaceFormatCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &surfaceFormatCount, details.presentModes.data());
	}

	return details;
}

void my_vulkan::VulkanDevice::createLogicalDevice(bool enableValidationLayer, const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions)
{
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	auto indices = queryQueueFamilyIndices(physicalDevice);
	std::set<uint32_t> queueIndices = { indices.graphicsAndComputeQueue.value(), indices.presentQueue.value() };
	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
	float priority = 1.0f;
	for (const auto index : queueIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = index;
		queueCreateInfo.pQueuePriorities = &priority;
		QueueCreateInfos.push_back(queueCreateInfo);
	}
	createInfo.queueCreateInfoCount = QueueCreateInfos.size();
	createInfo.pQueueCreateInfos = QueueCreateInfos.data();

	if (enableValidationLayer)
	{
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;
	createInfo.pEnabledFeatures = &deviceFeatures;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device!");
	vkGetDeviceQueue(device, indices.graphicsAndComputeQueue.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentQueue.value(), 0, &presentQueue);
}

void my_vulkan::VulkanDevice::destroyDevice()
{
	vkDestroyDevice(device, nullptr);
}

my_vulkan::VulkanDevice::~VulkanDevice()
{

}
