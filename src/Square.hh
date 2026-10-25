#pragma once

#include <SDL_rect.h>
#include "Piece.hh"

inline constexpr int BoardSize = 8;

struct Square
{
	SDL_Rect rect{};
	Piece piece{};
	int x = 0;
	int y = 0;
};
