#pragma once

#include <string>
#include "Sdl.hh"

namespace Texture
{
	// load an image with the current renderer; throws std::runtime_error on failure
	[[nodiscard]] sdl::Texture load(const std::string &path);
}
