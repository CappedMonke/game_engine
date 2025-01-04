#pragma once


class SDL_Window;


class Render_manager
{
public:

	bool start_up();
	void shut_down();
	void update();

private:

	SDL_Window* window = nullptr;
};