#include "Assets.hh"

#include <SDL.h>
#include <cstdlib>
#include <memory>
#include <system_error>
#include <vector>

#ifndef NX_CHESS_ASSET_DIR
#define NX_CHESS_ASSET_DIR "Assets"
#endif

namespace
{
	namespace fs = std::filesystem;

	// a file every asset directory has, to tell it apart from an unrelated "Assets" folder
	constexpr std::string_view MarkerFile = "wk.png";

	fs::path executableDirectory()
	{
		const std::unique_ptr<char, decltype(&SDL_free)> base{SDL_GetBasePath(), SDL_free};
		if (!base)
			return {};

		// SDL returns the directory with a trailing separator; drop it so parent_path() goes up a level
		fs::path dir{base.get()};
		return dir.has_filename() ? dir : dir.parent_path();
	}

	fs::path findDirectory()
	{
		std::vector<fs::path> candidates;

		if (const char *overridden = std::getenv("NX_CHESS_ASSET_DIR"))
			candidates.emplace_back(overridden);

		candidates.emplace_back(NX_CHESS_ASSET_DIR);

		if (const fs::path exe = executableDirectory(); !exe.empty())
		{
			candidates.push_back(exe / "Assets");
			candidates.push_back(exe.parent_path() / "Assets");
		}

		candidates.emplace_back("Assets");

		for (const fs::path &candidate : candidates)
		{
			std::error_code ec;
			if (fs::is_regular_file(candidate / MarkerFile, ec))
				return candidate;
		}

		// nothing found: report the build-time location in the load error
		return NX_CHESS_ASSET_DIR;
	}
}

const std::filesystem::path &Assets::directory()
{
	static const fs::path dir = findDirectory();
	return dir;
}

std::string Assets::path(std::string_view relative)
{
	return (directory() / relative).string();
}
