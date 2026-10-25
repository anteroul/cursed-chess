#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include "Game.hh"

namespace
{
	constexpr Uint32 TargetFps = 60;
	constexpr Uint32 FrameDelayMs = 1000 / TargetFps;

	// initializes SDL and its extension libraries for the lifetime of the object
	class SdlContext
	{
	public:
		SdlContext()
		{
			if (SDL_Init(SDL_INIT_VIDEO) != 0)
				throw std::runtime_error(std::string("SDL could not initialize: ") + SDL_GetError());

			// initialize PNG loading
			if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
				throw std::runtime_error(std::string("SDL_image could not initialize: ") + IMG_GetError());

			if (TTF_Init() != 0)
				throw std::runtime_error(std::string("SDL_ttf could not initialize: ") + TTF_GetError());
		}

		~SdlContext()
		{
			TTF_Quit();
			IMG_Quit();
			SDL_Quit();
		}

		SdlContext(const SdlContext &) = delete;
		SdlContext &operator=(const SdlContext &) = delete;
	};
}

int main(int /*argc*/, char * /*argv*/[])
{
	try
	{
		const SdlContext sdl;
		Game game;

		// Game loop:
		while (!game.shouldClose())
		{
			const Uint32 frameStart = SDL_GetTicks();

			game.updateGame();

			const Uint32 frameTime = SDL_GetTicks() - frameStart;
			if (frameTime < FrameDelayMs)
				SDL_Delay(FrameDelayMs - frameTime);
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
