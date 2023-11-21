#pragma once

#include <vector>
#include <vulkan/vulkan.h>
namespace my_vulkan
{
	class VulkanInstance
	{
	public:
		VulkanInstance(bool enableValidationLayer, const std::vector<const char*>& validationLayers);
		void createInstance(bool enableValidationLayer, const std::vector<const char*>& validationLayers);
		bool checkValidationLayerAvailability(const std::vector<const char*>& validationLayers);

		const VkInstance& getInstance() const { return instance; }

		void destroyInstance();
		~VulkanInstance();
	private:
		VkInstance instance;
	};
}