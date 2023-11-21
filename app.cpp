#include <iostream>
#include <memory>
#include "ImguiAPI.h"
#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "Arona.h"
#include "Camera.h"
#include "PointLight.h"

const std::vector<std::string> aronaTexturePaths = {
	"Models/arona/Arona_Body.png",
	"Models/arona/Arona_Eye.png" ,
	"Models/arona/Arona_Hair.png" ,
	"Models/arona/Arona_Halo.png",
	"Models/arona/Arona_Eyebrow.png",
	"Models/arona/Arona_Face.png"
	//"D:/GitHub/GAMES202/homework0/assets/mary/MC003_Kozakura_Mari.png"
};

const std::vector<std::string> aronaModelPaths = {
	"Models/arona/arona_body.obj",
	"Models/arona/arona_eye.obj" ,
	"Models/arona/arona_hair.obj" ,
	"Models/arona/arona_halo.obj",
	"Models/arona/arona_eyebrow.obj",
	"Models/arona/arona_face.obj"
	//"Models/floorY.obj"
	//"D:/GitHub/GAMES202/homework0/assets/mary/Marry.obj"
};

const std::vector<std::string> lightModelPaths = {
	//"D:/GitHub/GAMES202/homework0/assets/mary/Marry.obj"
	"Models/light.obj"
};
const std::vector<std::string> lightTexturePaths = 
{
	//"D:/GitHub/GAMES202/homework0/assets/mary/MC003_Kozakura_Mari.png"
	"Models/light.png"
};

int main()
{
	std::shared_ptr<my_vulkan::VulkanContext> context = std::make_shared<my_vulkan::VulkanContext>();
	std::shared_ptr<my_vulkan::VulkanRenderer> renderer = std::make_shared<my_vulkan::VulkanRenderer>(context.get());
	std::shared_ptr<my_vulkan::ImguiAPI> imgui = std::make_shared<my_vulkan::ImguiAPI>(context.get());
	auto fov = glm::radians(70.0f);
	auto as = 1920.0f / 1080.0f;
	auto x = glm::vec3(3, 3, 3);
	auto y = glm::vec3(0, 0, 0);

	std::shared_ptr<my_vulkan::Camera> camera = std::make_shared<my_vulkan::Camera>(fov, as, x, y, my_vulkan::CameraType::FIRST_PERSON);
	std::shared_ptr<my_vulkan::Arona> arona = std::make_shared<my_vulkan::Arona>("Arona", context.get(), aronaModelPaths, aronaTexturePaths);
	std::shared_ptr<my_vulkan::PointLight> light = std::make_shared<my_vulkan::PointLight>("Light", context.get(), lightModelPaths, lightTexturePaths);

	try
	{
		while (!glfwWindowShouldClose(context->wind.window))
		{
			glfwPollEvents();
			imgui->handleInput(context.get(), camera.get());
			arona->tick(renderer->getCurrentFrame(), camera.get());
			light->tick(renderer->getCurrentFrame(), camera.get());
			renderer->draw( context.get(), imgui.get(), {arona, light});
		}
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return EXIT_SUCCESS;
}



