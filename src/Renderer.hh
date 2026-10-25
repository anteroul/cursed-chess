#pragma once

#include <SDL.h>

// the renderer used for all drawing; it is owned by Window
namespace Renderer
{
	[[nodiscard]] SDL_Renderer *get() noexcept;
	void set(SDL_Renderer *renderer) noexcept;
	void setColor(Uint8 r, Uint8 g, Uint8 b) noexcept;
	void clear() noexcept;
	void present() noexcept;
	void fillRect(const SDL_Rect &rect) noexcept;
	void drawTexture(SDL_Texture *texture, const SDL_Rect &destination) noexcept;
}
