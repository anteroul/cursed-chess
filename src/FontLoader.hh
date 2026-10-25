#pragma once

#include <string>
#include "Sdl.hh"

namespace FontLoader
{
	// throws std::runtime_error on failure
	[[nodiscard]] sdl::Font load(const std::string &path, int pointSize);
}
