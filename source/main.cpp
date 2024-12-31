#define SDL_MAIN_USE_CALLBACKS
#define GAME_NAME    "Dawn's Ballad"
#define GAME_VERSION "0.0.0"
#define GAME_DOMAIN  "com.dawnsballad.www"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

#include "graphics/render_manager.hpp"


bool keep_running = true;

Render_manager render_manager = {};


SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	SDL_SetAppMetadata(GAME_NAME, GAME_VERSION, GAME_DOMAIN);

	// Init SDL
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_InitSubSystem error: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Init SDL window
	SDL_Window* window = SDL_CreateWindow(GAME_NAME, 800, 600, SDL_WINDOW_VULKAN);
	if (!window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow error: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Init Vulkan


	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
	if (!keep_running)
	{
		return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	SDL_Quit();
}