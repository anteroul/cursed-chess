#pragma once

#include <string>
#include "Sdl.hh"

// a line of text rendered once into a texture
class Text
{
public:
	Text(TTF_Font *font, const std::string &text, SDL_Color color, SDL_Point position);
	void render();

	SDL_Rect position{};

private:
	sdl::Texture texture;
};
