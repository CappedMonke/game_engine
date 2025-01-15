#pragma once

#include <vulkan/vulkan_core.h>


class SDL_Window;
class SDL_Surface;


class Render_manager
{
public:

	bool startup();
	void shutdown();
	void update();

private:

	SDL_Window*  window = nullptr;
	VkSurfaceKHR surface;
	VkInstance   vulkan_instance;
};