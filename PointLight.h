#pragma once
#include "Object.h"

namespace my_vulkan
{
	class PointLight : public Object
	{
	public:
		PointLight(const std::string& name, my_vulkan::VulkanContext* context, const std::vector<std::string>& modelPaths, const std::vector<std::string>& texturePaths);
		PointLight(const std::string& name, my_vulkan::VulkanContext* context, std::vector<std::string>&& modelPaths, std::vector<std::string>&& texturePaths);
		void tick(uint32_t currentImage, Camera* camera, PointLight* light) override;

		void setIntensity(float intensity) { this->intensity = intensity; }

		float intensity;
	};
}


