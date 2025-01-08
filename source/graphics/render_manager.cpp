#include "render_manager.hpp"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include "config/application.hpp"


bool Render_manager::start_up()
{
	window = SDL_CreateWindow(GAME_NAME, 800, 600, SDL_WINDOW_VULKAN);
	if (!window)
	{
		SDL_Log("Couldn't create window: %s", SDL_GetError());
		return false;
	}

	uint32_t           count_instance_extensions;
	const char* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);
	if (!instance_extensions)
	{
		SDL_Log("Couldn't get instance extensions: %s", SDL_GetError());
		return false;
	}

	VkInstanceCreateInfo create_info    = {};
	create_info.enabledExtensionCount   = count_instance_extensions;
	create_info.ppEnabledExtensionNames = instance_extensions;

	VkResult result = vkCreateInstance(&create_info, nullptr, &vulkan_instance);
	if (result != VK_SUCCESS)
	{
		SDL_Log("Couldn't create Vulkan instance: %d", result);
		return false;
	}

	return true;
}

void Render_manager::shut_down()
{
	SDL_DestroySurface(surface);
	SDL_DestroyWindow(window);
}

void Render_manager::update()
{
}
