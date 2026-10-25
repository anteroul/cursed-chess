#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace Assets
{
	// the Assets directory, found at runtime so moving the checkout doesn't break a build. Tried in order:
	// $NX_CHESS_ASSET_DIR, the source tree the game was built from, next to the executable,
	// one level above it (build directory), and the working directory
	[[nodiscard]] const std::filesystem::path &directory();

	// full path of a file inside the Assets directory, e.g. Assets::path("wk.png")
	[[nodiscard]] std::string path(std::string_view relative);
}
