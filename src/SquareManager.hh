#pragma once

#include "Square.hh"

namespace Sqr
{
	[[nodiscard]] constexpr bool onBoard(int x, int y) noexcept
	{
		return x >= 0 && x < BoardSize && y >= 0 && y < BoardSize;
	}

	// square at (x, y); the coordinates must be on the board
	[[nodiscard]] Square &at(int x, int y);

	// square at (x, y), or nullptr if the coordinates are off the board
	[[nodiscard]] Square *find(int x, int y) noexcept;
}
