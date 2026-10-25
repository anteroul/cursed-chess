cmake_minimum_required(VERSION 3.21)
project(cursed-chess LANGUAGES CXX)

# compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# default to a debug build, but let the user override it
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif ()

# prefer the CMake packages shipped with SDL, fall back to pkg-config
function(find_sdl_library name target)
	find_package(${name} CONFIG QUIET)
	if (NOT TARGET ${target})
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(${name} REQUIRED IMPORTED_TARGET GLOBAL ${name})
		add_library(${target} ALIAS PkgConfig::${name})
	endif ()
endfunction()

find_sdl_library(SDL2 SDL2::SDL2)
find_sdl_library(SDL2_image SDL2_image::SDL2_image)
find_sdl_library(SDL2_ttf SDL2_ttf::SDL2_ttf)

add_executable(cursed-chess
		src/main.cc
		src/Board.cc
		src/FontLoader.cc
		src/Game.cc
		src/GameManager.cc
		src/GUI.cc
		src/LegalMoves.cc
		src/Move.cc
		src/PieceFactory.cc
		src/Pieces.cc
		src/Renderer.cc
		src/ScreenSize.cc
		src/SquareManager.cc
		src/Text.cc
		src/Texture.cc
		src/Window.cc
		)

# pin the standard so the flag always reaches compile_commands.json (clangd)
set_target_properties(cursed-chess PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

if (MSVC)
	target_compile_options(cursed-chess PRIVATE /W4 /permissive-)
else ()
	target_compile_options(cursed-chess PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif ()

# assets are loaded from the source tree, so the binary works from any directory
target_compile_definitions(cursed-chess PRIVATE CURSED_CHESS_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Assets")

if (TARGET SDL2::SDL2main)
	target_link_libraries(cursed-chess PRIVATE SDL2::SDL2main)
endif ()
target_link_libraries(cursed-chess PRIVATE SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf)
