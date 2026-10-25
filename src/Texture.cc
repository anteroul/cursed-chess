#include "Texture.hh"

#include <SDL_image.h>
#include <stdexcept>
#include "Renderer.hh"

sdl::Texture Texture::load(const std::string &path)
{
	const sdl::Surface surface{IMG_Load(path.c_str())};
	if (!surface)
		throw std::runtime_error("Unable to load image " + path + ": " + IMG_GetError());

	// color key image
	SDL_SetColorKey(surface.get(), SDL_TRUE, SDL_MapRGB(surface->format, 0, 0xFF, 0xFF));

	sdl::Texture texture{SDL_CreateTextureFromSurface(Renderer::get(), surface.get())};
	if (!texture)
		throw std::runtime_error("Unable to create texture " + path + ": " + SDL_GetError());

	return texture;
}
