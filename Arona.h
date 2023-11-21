#pragma once
#include "Object.h"


namespace my_vulkan
{
	class Object;
	class Arona : public Object
	{
	public:
		Arona(const std::string& name, my_vulkan::VulkanContext* context, const std::vector<std::string>& modelPaths,
			const std::vector<std::string>& texturePaths);
		void tick(uint32_t currentImage, Camera* camera) override;
	};
}

