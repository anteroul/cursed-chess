#include "Text.hh"

#include <iostream>
#include "Renderer.hh"

Text::Text(TTF_Font *font, const std::string &text, SDL_Color color, SDL_Point pos, int wrapWidth)
	: position{pos.x, pos.y, 0, 0}
{
	const sdl::Surface surface{
		wrapWidth > 0 ? TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, static_cast<Uint32>(wrapWidth))
					  : TTF_RenderUTF8_Blended(font, text.c_str(), color)};
	if (!surface)
	{
		std::cerr << "Unable to render text \"" << text << "\": " << TTF_GetError() << '\n';
		return;
	}

	texture.reset(SDL_CreateTextureFromSurface(Renderer::get(), surface.get()));
	SDL_QueryTexture(texture.get(), nullptr, nullptr, &position.w, &position.h);
}

void Text::renderAt(int x, int y) const
{
	if (texture)
		Renderer::drawTexture(texture.get(), {x, y, position.w, position.h});
}

void Text::render() const
{
	if (texture)
		Renderer::drawTexture(texture.get(), position);
}
