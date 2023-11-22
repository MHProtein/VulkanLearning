#include "Arona.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "VulkanUniformBuffers.h"
#include "VulkanUtils.h"
#include "Camera.h"

my_vulkan::Arona::Arona(const std::string& name, my_vulkan::VulkanContext* context,const std::vector<std::string>& modelPaths, const std::vector<std::string>& texturePaths)
 : Object(name, context, modelPaths, texturePaths)
{

}

void my_vulkan::Arona::tick(uint32_t currentImage, Camera* camera, PointLight* light)
{
	Object::tick(currentImage, camera, light);
}
