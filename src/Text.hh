#pragma once

#include <string>
#include "Sdl.hh"

// text rendered once into a texture
class Text
{
public:
	// a wrapWidth above 0 breaks the text into lines at most that many pixels wide
	Text(TTF_Font *font, const std::string &text, SDL_Color color, SDL_Point position, int wrapWidth = 0);
	void render();

	SDL_Rect position{};

private:
	sdl::Texture texture;
};
