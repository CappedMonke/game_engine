#pragma once

#include <vulkan/vulkan_core.h>


class SDL_Window;
class SDL_Surface;


class Render_manager
{
public:

	bool start_up();
	void shut_down();
	void update();

private:

	SDL_Window*  window = nullptr;
	VkInstance   vulkan_instance;
	VkSurfaceKHR surface;
};