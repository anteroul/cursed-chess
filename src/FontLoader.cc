#include "FontLoader.hh"

#include <stdexcept>

sdl::Font FontLoader::load(const std::string &path, int pointSize)
{
	sdl::Font font{TTF_OpenFont(path.c_str(), pointSize)};
	if (!font)
		throw std::runtime_error("Unable to load font " + path + ": " + TTF_GetError());

	return font;
}
