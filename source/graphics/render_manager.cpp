#include "render_manager.hpp"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include "config/application.hpp"


bool Render_manager::startup()
{
	window = SDL_CreateWindow(GAME_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN & SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		return false;
	}

	VkApplicationInfo app_info  = {};
	app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName   = GAME_NAME;
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 0);
	app_info.pEngineName        = "No Engine";
	app_info.engineVersion      = VK_MAKE_API_VERSION(0, 0, 0, 0);
	app_info.apiVersion         = VK_API_VERSION_1_0;

	uint32_t           count_instance_extensions;
	const char* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

	VkInstanceCreateInfo create_info    = {};
	create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo        = &app_info;
	create_info.enabledExtensionCount   = count_instance_extensions;
	create_info.ppEnabledExtensionNames = instance_extensions;
	create_info.enabledLayerCount       = 0;

	VkResult result = vkCreateInstance(&create_info, nullptr, &vulkan_instance);
	if (result != VK_SUCCESS)
	{
		SDL_SetError("Failed to create Vulkan instance: %d", result);
		return false;
	}

	return true;
}

void Render_manager::shutdown()
{
	vkDestroyInstance(vulkan_instance, nullptr);
	SDL_DestroyWindow(window);
}

void Render_manager::update()
{
}