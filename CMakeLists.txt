cmake_minimum_required(VERSION 3.25)
project(nx-chess VERSION 1.0 LANGUAGES CXX)

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

# find shared libraries (libnexilis) relative to the executable, so the build survives moving the checkout
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# Nexilis networking library (git submodule), used for multiplayer games
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/nexilis/nexilis/CMakeLists.txt")
	message(FATAL_ERROR "Nexilis is missing. Run: git submodule update --init --recursive")
endif ()
# Nexilis' public headers include OpenSSL. Under Nix the compiler wrapper supplies that include path,
# so CMake treats it as implicit and leaves it out of compile_commands.json; list it explicitly for clangd.
find_package(OpenSSL 1.1.1 REQUIRED)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})

add_subdirectory(external/nexilis/nexilis nexilis EXCLUDE_FROM_ALL SYSTEM)

add_executable(nx-chess
		src/main.cc
		src/Assets.cc
		src/Board.cc
		src/FontLoader.cc
		src/Game.cc
		src/GameManager.cc
		src/GUI.cc
		src/LegalMoves.cc
		src/Network.cc
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
set_target_properties(nx-chess PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

if (MSVC)
	target_compile_options(nx-chess PRIVATE /W4 /permissive-)
else ()
	target_compile_options(nx-chess PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif ()

# preferred asset location; Assets.cc falls back to paths relative to the executable
target_compile_definitions(nx-chess PRIVATE NX_CHESS_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Assets"
		NX_CHESS_VERSION="${PROJECT_VERSION}")

if (TARGET SDL2::SDL2main)
	target_link_libraries(nx-chess PRIVATE SDL2::SDL2main)
endif ()
target_link_libraries(nx-chess PRIVATE SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf nexilis)
