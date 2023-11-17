#include <iostream>

#include "FirstApp.h"

int main()
{
	my_vulkan::FirstApp app{};


	try
	{
		app.run();
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return EXIT_SUCCESS;
}



