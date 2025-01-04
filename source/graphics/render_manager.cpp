#include "render_manager.hpp"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <config/application.hpp>


bool Render_manager::start_up()
{
	window = SDL_CreateWindow(GAME_NAME, 800, 600, SDL_WINDOW_VULKAN);
	if (!window)
	{
		SDL_Log("Couldn't create window: %s", SDL_GetError());
		return false;
	}
}

void Render_manager::shut_down()
{
}

void Render_manager::update()
{
}
