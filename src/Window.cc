#include "Window.hh"

#include <stdexcept>
#include <string>
#include "Renderer.hh"
#include "ScreenSize.hh"

Window::Window()
	: window{SDL_CreateWindow("Chess", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
							  SDL_WINDOW_RESIZABLE)}
{
	if (!window)
		throw std::runtime_error(std::string("Could not create window: ") + SDL_GetError());

	renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	if (!renderer)
		throw std::runtime_error(std::string("Could not create renderer: ") + SDL_GetError());

	Renderer::set(renderer.get());
	publishSize();
}

void Window::handleEvent(const SDL_Event &e)
{
	if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED)
	{
		// get new screensize and set it accordingly
		SDL_GetWindowSize(window.get(), &width, &height);
		publishSize();
	}
}

void Window::publishSize() const noexcept
{
	Screen::setWidth(width);
	Screen::setHeight(height);
}
