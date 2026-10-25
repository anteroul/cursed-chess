#pragma once

#include "Sdl.hh"

// the application window; owns the renderer published through Renderer::get()
class Window
{
public:
	// throws std::runtime_error on failure
	Window();
	void handleEvent(const SDL_Event &e);

private:
	void publishSize() const noexcept;

	int width = 800;
	int height = 600;
	sdl::Window window;
	sdl::Renderer renderer;
};
