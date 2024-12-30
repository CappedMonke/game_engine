#include "graphics/render_manager.hpp"


Render_manager render_manager;

bool exit_game = false;


int main(int argc, char* argv[])
{
	// start up managers
	render_manager.start_up();


	while (!exit_game)
	{
	}

	// shut down managers
	render_manager.shut_down();

	return 0;
}