#include "PointLight.h"

my_vulkan::PointLight::PointLight(const std::string& name, my_vulkan::VulkanContext* context,
                                  const std::vector<std::string>& modelPaths, const std::vector<std::string>& texturePaths)
	: Object(name, context, modelPaths, texturePaths)
{

}

my_vulkan::PointLight::PointLight(const std::string& name, my_vulkan::VulkanContext* context,
	std::vector<std::string>&& modelPaths, std::vector<std::string>&& texturePaths)
	: Object(name, context, std::move(modelPaths), std::move(texturePaths))
{

}

void my_vulkan::PointLight::tick(uint32_t currentImage, Camera* camera, PointLight* light)
{
	Object::tick(currentImage, camera, light);
}
