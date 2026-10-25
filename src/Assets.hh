#pragma once

#include <string>
#include <string_view>

#ifndef CURSED_CHESS_ASSET_DIR
#define CURSED_CHESS_ASSET_DIR "Assets"
#endif

namespace Assets
{
	inline constexpr std::string_view directory = CURSED_CHESS_ASSET_DIR;

	// full path of a file inside the Assets directory, e.g. Assets::path("wk.png")
	[[nodiscard]] inline std::string path(std::string_view relative)
	{
		std::string result{directory};
		result += '/';
		result += relative;
		return result;
	}
}
