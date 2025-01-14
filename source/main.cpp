#define SDL_MAIN_USE_CALLBACKS


#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>

#include "config/application.hpp"
#include "graphics/render_manager.hpp"


// =============================================================================
// Globals
// =============================================================================
Render_manager render_manager;


// =============================================================================
// SDL Callbacks
// =============================================================================
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	SDL_SetAppMetadata(GAME_NAME, GAME_VERSION, GAME_DOMAIN);

	// Init SDL
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		SDL_Log("SDL_InitSubSystem error: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Start up render manager
	if (!render_manager.start_up())
	{
		SDL_Log("Render_manager::start_up error %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
	render_manager.update();

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	if (event->type == SDL_EVENT_QUIT)
	{
		return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	render_manager.shut_down();
}