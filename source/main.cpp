#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

#include "graphics/render_manager.hpp"


Render_manager render_manager;


SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	SDL_SetAppMetadata("Dawn's Ballad", "0.0.0", "com.dawnsballad.www");
	SDL_Init(0);

	SDL_CreateWindow("Dawn's Ballad", 100, 100, SDL_WINDOW_FULLSCREEN);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
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