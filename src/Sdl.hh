#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <memory>

// owning RAII handles for SDL resources
namespace sdl
{
	// adapts an SDL destroy function into a std::unique_ptr deleter
	template <auto DestroyFn>
	struct Deleter
	{
		template <typename T>
		void operator()(T *ptr) const noexcept
		{
			DestroyFn(ptr);
		}
	};

	using Window = std::unique_ptr<SDL_Window, Deleter<SDL_DestroyWindow>>;
	using Renderer = std::unique_ptr<SDL_Renderer, Deleter<SDL_DestroyRenderer>>;
	using Texture = std::unique_ptr<SDL_Texture, Deleter<SDL_DestroyTexture>>;
	using Surface = std::unique_ptr<SDL_Surface, Deleter<SDL_FreeSurface>>;
	using Font = std::unique_ptr<TTF_Font, Deleter<TTF_CloseFont>>;
}
