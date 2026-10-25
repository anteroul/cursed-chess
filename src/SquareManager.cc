#include "SquareManager.hh"

#include <array>
#include <cassert>

namespace
{
	using Grid = std::array<std::array<Square, BoardSize>, BoardSize>;

	Grid makeGrid()
	{
		Grid grid{};
		for (int x = 0; x < BoardSize; x++)
			for (int y = 0; y < BoardSize; y++)
			{
				grid[x][y].x = x;
				grid[x][y].y = y;
			}
		return grid;
	}

	// the actual squares, indexed [x][y]
	Grid squares = makeGrid();
}

Square &Sqr::at(int x, int y)
{
	assert(onBoard(x, y));
	return squares[x][y];
}

Square *Sqr::find(int x, int y) noexcept
{
	return onBoard(x, y) ? &squares[x][y] : nullptr;
}
