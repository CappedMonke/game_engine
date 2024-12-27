#include <iostream>
#include <vulkan/vulkan.hpp>


int main()
{
	VkInstance instance;

	VkApplicationInfo application_info = {};
	application_info.sType             = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName  = "engine";
	application_info.engineVersion     = VK_MAKE_API_VERSION(0, 1, 0, 0);
	application_info.apiVersion        = VK_API_VERSION_1_0;

	const std::vector<char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};

	VkInstanceCreateInfo create_info = {};
	create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo     = &application_info;

	if (!validation_layers.empty())
	{
		create_info.enabledLayerCount   = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	}

	if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
	{
		std::cerr << "Failed to create Vulkan instance!" << std::endl;
		return -1;
	}

	std::cout << "Vulkan instance created successfully!" << std::endl;

	vkDestroyInstance(instance, nullptr);

	return 0;
}