#include "Renderer.hh"

namespace
{
	SDL_Renderer *renderer = nullptr;
}

SDL_Renderer *Renderer::get() noexcept
{
	return renderer;
}

void Renderer::set(SDL_Renderer *newRenderer) noexcept
{
	renderer = newRenderer;
}

void Renderer::setColor(Uint8 r, Uint8 g, Uint8 b) noexcept
{
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
}

void Renderer::clear() noexcept
{
	SDL_RenderClear(renderer);
}

void Renderer::present() noexcept
{
	SDL_RenderPresent(renderer);
}

void Renderer::fillRect(const SDL_Rect &rect) noexcept
{
	SDL_RenderFillRect(renderer, &rect);
}

void Renderer::drawTexture(SDL_Texture *texture, const SDL_Rect &destination) noexcept
{
	SDL_RenderCopy(renderer, texture, nullptr, &destination);
}
